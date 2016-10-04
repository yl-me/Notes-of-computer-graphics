#include "Physics1.h"

// An object to represent a spring with inner friction binding two masses.
// The spring has a normal length (The length that the spring does not exert any force)
class Spring {
public:
	Mass* mass1;
	Mass* mass2;

	float springConstant;             // A constant to represent the stiffness of the spring
	float springLength;
	float frictionConstant;

	// Constructor
	Spring(Mass* mass1, Mass* mass2, float springConstant, float springLength, float frictionConstant) :
		mass1(mass1), mass2(mass2), springConstant(springConstant), springLength(springLength), 
		frictionConstant(frictionConstant) {}

	void solve()
	{
		Vector3D springVector = mass1->pos - mass2->pos;
		float r = springVector.length();
		Vector3D force;

		if (r != 0) {
			force += (springVector / r) * (r - springLength) * (-springConstant);
		}
		force += -(mass1->vel - mass2->vel) * frictionConstant;

		mass1->applyForce(force);
		mass2->applyForce(-force);
	}
};

class RopeSimulation : public Simulation {
public:
	Spring** springs;

	Vector3D gravitation;
	Vector3D ropeConnectionPos;
	Vector3D ropeConnectionVel;
	
	// A constant to represent how much the ground shall repel the masses
	float groundRepulsionConstant;
	// A constant of friction applied to masses by the ground (sliding of rope)
	float groundFrictionConstant;
	// A constant of absorption friction alppiled to masses by the ground (vertical collisions of rope)
	float groundAbsorptionConstant;

	float groundHeight;
	float airFrictionConstant;              // A constant of air friction-applied to masses

	RopeSimulation(
		int numOfMasses,
		float m,
		float springConstant,
		float springLength,
		float springFrictionConstant,
		Vector3D gravitation,
		float airFrictionConstant,
		float groundRepulsionConstant,
		float groundFrictionConstant,
		float groundAbsorptionConstant,
		float groundHeight
		) : Simulation(numOfMasses, m) {
		this->gravitation = gravitation;
		this->airFrictionConstant = airFrictionConstant;
		this->groundFrictionConstant = groundFrictionConstant;
		this->groundRepulsionConstant = groundRepulsionConstant;
		this->groundAbsorptionConstant = groundAbsorptionConstant;
		this->groundHeight = groundHeight;

		for (int i = 0; i < numOfMasses; ++i) {
			masses[i]->pos.x = i * springLength;
			masses[i]->pos.y = 0;
			masses[i]->pos.z = 0;
		}
		springs = new Spring*[numOfMasses - 1];

		for (int i = 0; i < numOfMasses - 1; ++i) {
			springs[i] = new Spring(masses[i], masses[i + 1], springConstant, springLength, 
				springFrictionConstant);
		}
	}

	void release()
	{
		Simulation::release();
		for (int i = 0; i < numOfMasses - 1; ++i) {
			delete(springs[i]);
			springs[i] = NULL;
		}
		delete(springs);
		springs = NULL;
	}

	void solve()
	{
		for (int i = 0; i < numOfMasses - 1; ++i) {
			springs[i]->solve();
		}
		for (int i = 0; i < numOfMasses; ++i) {
			masses[i]->applyForce(gravitation * masses[i]->m);          // Gravitation
			masses[i]->applyForce(-masses[i]->vel * airFrictionConstant);    // Air Friction

			if (masses[i]->pos.y < groundHeight) {
				Vector3D v = masses[i]->vel;             // Velocity
				v.y = 0;                                 // Omit the velocity component in y direction

				// The velocity in y direction is omited because we will apply a friction force to create
				// a sliding effect. Sliding is parallel to the ground. Velocity in y direction will be used
				// in the absorption effect.
				masses[i]->applyForce(-v * groundFrictionConstant);
				v = masses[i]->vel;
				v.x = 0;               // Omit
				v.z = 0;

				if (v.y < 0) {      // Absorb energy only when a mass collides towards the ground
					masses[i]->applyForce(-v * groundAbsorptionConstant);
				}

				// The ground shall repel a mass like a spring.
				Vector3D force = Vector3D(0, groundRepulsionConstant, 0) * (groundHeight - masses[i]->pos.y);
				masses[i]->applyForce(force);
			}
		}
	}

	void simulate(float dt)              // Overriden
	{
		Simulation::simulate(dt);
		ropeConnectionPos += ropeConnectionVel * dt;   // Iterate the positon of ropeConnectionPos

		if (ropeConnectionPos.y < groundHeight) {
			ropeConnectionPos.y = groundHeight;
			ropeConnectionVel.y = 0;
		}

		masses[0]->pos = ropeConnectionPos;
		masses[0]->vel = ropeConnectionVel;
	}

	void setRopeConnectionVel(Vector3D ropeConnectionVel)
	{
		this->ropeConnectionVel = ropeConnectionVel;
	}
};
