#include <math.h>

class Vector3D {
public:
    float x, y, z;

    Vector3D(): x(0), y(0), z(0) {}
    Vector3D(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Vector3D& operator= (Vector3D v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    Vector3D operator+ (Vector3D v)
    {
        return Vector3D(x + v.x, y + v.y, z + v.z);
    }

    Vector3D operator- (Vector3D v)
    {
        return Vector3D(x - v.x, y - v.y, z - v.z);
    }

    Vector3D operator* (float value)
    {
        return Vector3D(x * value, y * value, z * value);
    }

    Vector3D operator/ (float value)
    {
        return Vector3D(x / value, y / value, z / value);
    }

    Vector3D& operator+= (Vector3D v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vector3D& operator-= (Vector3D v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vector3D& operator*= (Vector3D v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }

    Vector3D& operator/= (Vector3D v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }

    Vector3D operator- ()
    {
        return Vector3D(-x, -y, -z);
    }

    float length()
    {
        return sqrtf(x*x + y*y + z*z);
    }

    void unitize()                            // Normalizes
    {
        float length = this->length();
        if (length == 0) return;

        x /= length;
        y /= length;
        z /= length;
    }

    Vector3D unit()                        // Normalizes return a new Vector3D
    {
        float length = this->length();
        if (length == 0)
            return *this;
        return Vector3D(x / length, y / length, z / length);
    }
};

class Mass {
public:
    float m;                 // The mass value
    Vector3D pos;            // Position
    Vector3D vel;            // Velocity
    Vector3D force;          // Force

    Mass(float m): m(m) {}

    void applyForce(Vector3D force)
    {
        this->force += force;
    }

    void init()
    {
        force.x = 0;
        force.y = 0;
        force.z = 0;
    }

    void simulate(float dt)                 // New velocity and position
    {
        vel += (force / m) * dt;
        pos += vel * dt;
    }
};

class Simulation {
public:
    int    numOfMasses;
    Mass** masses;

    Simulation(int numOfMasses, float m)                 // Constructor
    {
        this->numOfMasses = numOfMasses;
        masses = new Mass*[numOfMasses];
        for (int i = 0; i < numOfMasses; ++i) {
            masses[i] = new Mass(m);
        }
    }

    virtual void release()                      // Delete
    {
        for (int i = 0; i < numOfMasses; ++i) {
            delete(masses[i]);
            masses[i] = NULL;
        }
        delete(masses);
        masses = NULL;
    }

    Mass* getMass(int index)
    {
        if (index < 0 || index >= numOfMasses) {
            return NULL;
        }
        return masses[index];
    }

    virtual void init()
    {
        for (int i = 0; i < numOfMasses; ++i) {
            masses[i]->init();
        }
    }
    // No implementation because no forces are wanted in this basic container
    // in advanced containers, this method will be overrided and some forces will act on masses
    virtual void solve() {}

    virtual void simulate(float dt)
    {
        for (int i = 0; i < numOfMasses; ++i) {
            masses[i]->simulate(dt);
        }
    }

    virtual void operate(float dt)                     // The complete produce of simulation
    {
        init();
        solve();
        simulate(dt);
    }
};

class ConstantVelocity : public Simulation {
public:
    ConstantVelocity() : Simulation(1, 1.0f)
    {
        masses[0]->pos = Vector3D(0.0f, 0.0f, 0.0f);
        masses[0]->vel = Vector3D(1.0f, 0.0f, 0.0f);
    }
};

class MotionUnderGravitation : public Simulation {
public:
    Vector3D gravitation;
    MotionUnderGravitation(Vector3D gravitation) : Simulation(1, 1.0f)
    {
        this->gravitation = gravitation;
        masses[0]->pos = Vector3D(-10.0f, 0.0f, 0.0f);
        masses[0]->vel = Vector3D(10.0f, 15.0f, 0.0f);
    }

    virtual void solve()
    {
        for (int i = 0; i < numOfMasses; ++i) {
            masses[i]->applyForce(gravitation * masses[i]->m);
        }
    }
};

class MassConnectedWithSpring : public Simulation {
public:
    float springConstant;                      // More the springConstant, stiffer the spring force
    Vector3D connectionPos;                    // The arbitrary constant point that the mass is connected

    MassConnectedWithSpring(float springConstant) :Simulation(1, 1.0f)
    {
        this->springConstant = springConstant;
        connectionPos = Vector3D(0.0f, -5.0f, 0.0f);
        masses[0]->pos = connectionPos + Vector3D(10.0f, 0.0f, 0.0f);
        masses[0]->vel = Vector3D(0.0f, 0.0f, 0.0f);
    }

    virtual void solve()
    {
        for (int i = 0; i < numOfMasses; ++i) {
            Vector3D springVector = masses[i]->pos - connectionPos;
            masses[i]->applyForce(-springVector * springConstant);
        }
    }
};