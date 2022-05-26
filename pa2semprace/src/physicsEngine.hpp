#pragma once
#include "linearAlgebra.hpp"
#include "physicsAttributes.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <queue>
#include <numeric>
#include <set>

class CObject;
class CPhysicsObject;
class CForceField
{
public:
  explicit CForceField( std::function<void(CPhysicsObject &)> );
  void applyForce( CPhysicsObject & ) const;
  static CForceField gravitationalField( double g = 10 );
  std::function<void(CPhysicsObject &)> m_fieldFunctor;
};

struct TContactPoint
{
  TVector<2> overlapVector; // distance needed to push second object
  TVector<2> contactPoint;
};

struct TManifold
{
  TManifold( CObject *first, CObject *second );

  TManifold( CObject *first, CObject *second,
             TVector<2> overlapVector,
             TVector<2> contactPoint );

  bool isValid() const
  {
    return first && second;
  }

  CPhysicsObject *first;
  CPhysicsObject *second;
  std::vector<TContactPoint> contacts;
};

class CPhysicsEngine
{
public:
  void step( std::vector<CObject *> &, double );
  void addField( CForceField );
  void registerCollisionCallback( const std::function<void(std::vector<TManifold>)> &callback )
  {
    m_collisionCallback = callback;
  }
private:
  void accumulateForces( std::vector<CObject *> & );
  static void applyForces( std::vector<CObject *> &, double );
  static std::vector<TManifold> findCollisions( std::vector<CObject *> & );
  static void resolveCollisions( std::vector<TManifold> & );
  static void applyImpulses( std::vector<TManifold> & );
  static void applyImpulse( const TManifold & );
  static void applyImpulse( const TManifold &, const TContactPoint & );
  static TVector<2> getNormalImpulse( const TManifold &, const TContactPoint & );
  static TVector<2> getFrictionImpulse( const TManifold &, const TContactPoint & );
  static TVector<2> getRelativeVelocity( const CPhysicsObject &,
                                         const CPhysicsObject &,
                                         const TVector<2> &pointOfContact );
  static double getCombinedInvMass( const CPhysicsObject &,
                                    const CPhysicsObject &,
                                    const TVector<2> &point,
                                    const TVector<2> &direction );

  static double getObjectInvMass( const CPhysicsObject &,
                                  const TVector<2> &point,
                                  const TVector<2> &direction );

  std::vector<CForceField> m_fields;
  std::function<void(std::vector<TManifold>)> m_collisionCallback;
};
