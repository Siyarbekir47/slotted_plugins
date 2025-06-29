#include "sdk_spell.hpp"
#include "cpplinq.h"

using namespace cpplinq;

void SDKSpell::SetSkillshot(
    float _delay,
    float _radius,
    float _speed,
    ESpellType _type,
    bool _isCollision,
    ECollisionFlags _collisionFlags,
    bool _forceAddRadius
)
{
    this->delay = _delay;
    this->radius = _radius;
    this->speed = _speed;
    this->type = type;
    this->collisionFlags = _isCollision ? _collisionFlags : ECollisionFlags::none;
    this->rangeAddRadius = _forceAddRadius;
}

#pragma region Basic 
auto SDKSpell::IsReady( float time ) -> bool
{
    if ( get_time( ) - lastCastAttempt <= 3.f * TICK )
        return false;

    if ( inst )
    {
        if ( time <= -1.f )
            return inst->is_ready( );
        else
            return inst->is_learned( ) && inst->cooldown_expire( ) - get_time( ) <= time;
    }

    return false;
}

u32 SDKSpell::Level( )
{
    return inst ? inst->level( ) : 0;
}

float SDKSpell::ManaCost( )
{
    return inst ? inst->manacost( ) : 0.f;
}

auto SDKSpell::IsHexFlash( ) -> bool
{
    return inst && buff_hash_rt( inst->name( ) ) ==
        buff_hash( "SummonerFlashPerksHextechFlashtraptionV2" ); // hash_fnv1a
}

auto SDKSpell::RealRange( ) -> float
{
    return range + ( this->type == ESpellType::circle ? this->radius : 0.f );
}

auto SDKSpell::SetFrom( Vec3 f ) -> void
{
    vecFrom = f;
}

std::string SDKSpell::GetName( )
{
    if ( inst )
        return inst->name( );

    return "";
}

u32 SDKSpell::GetHash( )
{
    if ( inst )
        return buff_hash_rt( inst->name( ) );

    return 0u;
}

#pragma endregion

#pragma region Prediction
auto SDKSpell::GetPrediction( Object* tar, float overrideRange, int overrideHC, bool ignoreCollision, float overrideDelay, float overrideSpeed ) -> PredictionResult
{
    auto predResult = PredictionResult( );
    if ( !tar )
        return predResult;

    if ( tar->is_hero( ) )
    {
        predResult = prediction::predict_skillshot(
            tar,
            ( overrideRange == -1 ) ? range : overrideRange,
            overrideSpeed <= -1.f ? this->speed : overrideSpeed,
            radius,
            overrideDelay <= -1.f ? this->delay : overrideDelay,
            vecFrom,
            this->rangeAddRadius );
    }
    else
    {
        auto totalDelay = ( overrideDelay <= -1.f ? this->delay : overrideDelay ) + tar->position( ).dist_to( object_manager::get_local( )->position( ) ) / ( overrideSpeed <= -1.f ? this->speed : overrideSpeed );
        auto totalRange = overrideRange <= -1.f ? ( this->RealRange( ) + ( this->rangeAddRadius ? tar->get_bounding_radius( ) : 0.f ) ) : overrideRange;

        auto posAfter = prediction::simple_predict( tar, totalDelay );
        if ( posAfter.has_value( ) &&
            ( vecFrom.is_valid( ) ? vecFrom : object_manager::get_local( )->position( ) ).dist_to( *posAfter ) < totalRange )
        {
            predResult.valid = true;
            predResult.position = predResult.default_position = posAfter.value( );
            predResult.hitchance = EHitchance::very_high;
        }
    }


    if ( !predResult.valid ||
        predResult.hitchance < ( overrideHC <= -1 ? minHitchance : ( EHitchance )overrideHC ) ||
        predResult.hitchance >= EHitchance::invalid )
    {
        predResult.valid = false;
    }

    if ( !ignoreCollision && IsCollision( tar, predResult.position ) )
    {
        predResult.valid = false;
    }

    SetFrom( );

    return predResult;
}

auto SDKSpell::IsCollision( Object* tar, Vec3& to, std::optional<ECollisionFlags> flags ) -> bool
{
    auto vFrom{ vecFrom.is_valid( ) ? vecFrom : object_manager::get_local( )->position( ) };
    SetFrom( );

    if ( flags.value_or( collisionFlags ) & ECollisionFlags::spell_wall )
    {
        if ( utils::is_windwall_in_line( vFrom, to ) ) {
            return true;
        }
    }

    if ( flags.value_or( collisionFlags ) & ECollisionFlags::minions ) 
    {
        if ( prediction::is_minion_in_line_predicted( vFrom, to, this->radius, this->delay, this->speed ) )
            return true;
    }

    return false;
}
#pragma endregion

#pragma region Cast
bool SDKSpell::Cast( Object* tar, int _hc )
{
    auto predResult{ GetPrediction( tar, -1.f, _hc ) };
    if ( predResult.valid )
    {
        return Cast( predResult.position );
    }

    return false;
}

bool SDKSpell::Cast( Object* tar, EHitchance _hc )
{
    return Cast( tar, ( int )_hc );
}

bool SDKSpell::CastAOE( int min_count )
{
    auto pos = utils::get_heroes_multihit_position( range, speed, radius, delay, this->type == ESpellType::linear, min_count );
    if ( pos.is_valid( ) )
        return Cast( pos );

    return false;
}

bool SDKSpell::CastFarm( int countMinions, bool isJungle, int aaCount )
{
    auto farm_position{ utils::get_best_clear_pos( delay, radius,range, speed, isJungle, static_cast<int>( this->type ), aaCount, countMinions ) };
    if ( farm_position.is_valid( ) )
    {
        lastCastAttempt = get_time( );
        return this->Cast( farm_position );
    }

    return false;
}

bool SDKSpell::CastOnInterruptTarget( float overrideRange, bool cast_on_unit, int min_danger_level )
{
    auto totalRange{ overrideRange <= 0.f ? RealRange( ) : overrideRange };
    auto vFrom{ vecFrom.is_valid( ) ? vecFrom : object_manager::get_local( )->position( ) };

    auto enemies = from( object_manager::get_enemy_heroes( ) )
        >> where( [ & ]( Object* x )
    {
        if ( !x ||
            target_selector::is_bad_target( x ) || x->ignore_cc( ) ||
            vFrom.dist_to( x->position( ) ) > ( totalRange + this->rangeAddRadius ? x->get_bounding_radius( ) : 0.f ) )
            return false;

        if ( utils::is_casting_interruptible_spell( x ) >= min_danger_level )
            return true;

        return false;
    } )
        >> orderby_descending( [ & ]( Object* x )
    {
        return utils::is_casting_interruptible_spell( x );
    } )
        >> to_vector( );


    for ( auto& x : enemies )
    {
        if ( !cast_on_unit )
        {
            if ( this->Cast( x, EHitchance::low ) )
            {
                SetFrom( );
                return true;
            }
        }
        else
        {
            if ( !this->IsCollision( x, vecFrom ) && this->CastOn( x ) )
            {
                SetFrom( );
                return true;
            }
        }
    }

    SetFrom( );
    return false;
}

bool SDKSpell::CastDir( Vec3& pos, Vec3& pos2 )
{
    return spellbook::cast_spell_direction( slot, pos, pos2 );
}

bool SDKSpell::Release( Vec3& pos )
{
    lastCastAttempt = get_time( );

    return spellbook::update_chargeable_spell( slot, pos );
}

bool SDKSpell::Cast( Vec3& pos )
{
    lastCastAttempt = get_time( );

    if ( pos.is_valid( ) )
    {
        return spellbook::cast_spell_to_position( slot, pos );
    }
    else
    {
        return spellbook::cast_spell( slot );
    }

    return false;
}

bool SDKSpell::CastOn( Object* tar )
{
    lastCastAttempt = get_time( );

    return spellbook::cast_spell_on_target( slot, tar->network_id( ) );
}
#pragma endregion

#pragma region TS
Object* SDKSpell::GetTarget( float overrideRange, std::string const& configWhitelistBase )
{
    auto vFrom{ vecFrom.is_valid( ) ? vecFrom : object_manager::get_local( )->position( ) };
    auto tar = target_selector::get_primary_target( );
    if ( !tar || vFrom.dist_to( tar->position( ) ) > this->RealRange( ) || 
        ( !configWhitelistBase.empty( ) && !menu_api::cfg_get<bool>( fnv1a64_rt( configWhitelistBase + tar->object_name( ) ) ) )
        )
    {
        tar = target_selector::get_secondary_target( );
        if ( !tar || vFrom.dist_to( tar->position( ) ) > this->RealRange( ) ||
            ( !configWhitelistBase.empty( ) && !menu_api::cfg_get<bool>( fnv1a64_rt( configWhitelistBase + tar->object_name( ) ) ) )
            )
            return nullptr;

        return tar;
    }

    return tar;
}

Object* SDKSpell::GetClosestTargetAntiMelee( float overrideRange, std::string whitelistbase ) {
    auto totalRange{ overrideRange <= 0.f ? RealRange( ) : overrideRange };

    auto result = from( object_manager::get_enemy_heroes( ) )
        >> where( [ & ]( Object* x )
    {
        if ( x && object_manager::get_local( )->position( ).dist_to( x->position( ) ) < ( totalRange + ( this->rangeAddRadius ? x->get_bounding_radius( ) : 0.f ) ) &&
            !target_selector::is_bad_target( x ) && x->is_melee( ) && ( whitelistbase.empty( ) || menu_api::cfg_get<bool>( fnv1a64_rt( whitelistbase + x->object_name( ) ) ) ) )
        {
            auto pos{ prediction::simple_predict( x, 0.2f ) };
            if ( pos.has_value( ) && ( *pos ).dist_to( object_manager::get_local( )->position( ) ) < x->attack_range( ) + object_manager::get_local( )->get_bounding_radius( ) + x->get_bounding_radius( ) + 5.f ) {
                return true;
            }
        }
        return false;
    } ) >>
        orderby_ascending( [ & ]( Object* x ) {
        return x->position( ).dist_to( object_manager::get_local( )->position( ) );
    } ) >>
        to_vector( );

    SetFrom( );

    return result.empty( ) ? nullptr : result.front( );
}

Object* SDKSpell::GetClosestTarget( float overrideRange )
{
    auto totalRange{ overrideRange <= 0.f ? RealRange( ) : overrideRange };
    auto vFrom{ vecFrom.is_valid( ) ? vecFrom : object_manager::get_local( )->position( ) };

    auto result = from( object_manager::get_enemy_heroes( ) ) >> where( [ & ]( Object* x ) {
        return x && vFrom.dist_to( x->position( ) ) < ( totalRange + ( this->rangeAddRadius ? x->get_bounding_radius( ) : 0.f ) ) &&
            !target_selector::is_bad_target( x );
    } ) >>
        orderby_ascending( [ & ]( Object* x ) {
        return x->position( ).dist_to( vFrom );
    } ) >>
        to_vector( );

    SetFrom( );

    return result.empty( ) ? nullptr : result.front( );
}
#pragma endregion