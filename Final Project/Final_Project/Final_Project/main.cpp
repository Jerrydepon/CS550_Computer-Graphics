
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include <GL/glew.h>

#elif __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#endif

#include <time.h>
#include <sys/time.h>
#define PI 3.14159265
#include <unistd.h>
#include <vector>

#define renderstate unsigned int
#define fountain 0
#define cannon 1
#define rain 2

#define camerastate unsigned int
#define out 0

#define SPHERE_RADIUS   1.5
#define SPHERE_SLICES   50
#define SPHERE_STACKS   50

//    Author:            Chih-Hsiang Wang

// title of these windows:

const char *WINDOWTITLE = { "Final_Project - Chih-Hsiang Wang" };
const char *GLUITITLE   = { "User Interface Window" };

using namespace std;

struct timeval point, current, tfrozen, thawed, instant;
double pTime = 0;
bool paused = false;
bool frozen = false;
float tspacing = 0.02;
float pauseOffset = 0;
float p = 0;
camerastate cstate = out;

double preciseDiff(struct timeval & t1, struct timeval & t2)
{
    long sec = t1.tv_sec - t2.tv_sec;
    long usec = t1.tv_usec - t2.tv_usec;
    
    return((((sec) * 1000 + usec/1000.0) + 0.5)/2500);
}

float toRadians(float angle)
{
    return (angle*PI/180);
}

class Cannon{
public:
    
    void setAngle(float _angle)
    {
        angle = _angle;
        if(angle > 90)
            angle = 90;
        if(angle < 0)
            angle = 0;
    }
    float getAngle()
    {
        return angle;
    }
    float getRadius()
    {
        return radius;
    }
    float getX()
    {
        return x;
    }
    float getY()
    {
        return y;
    }
    float getZ()
    {
        return z;
    }
    float getLength()
    {
        return length;
    }
    void draw()
    {
        glPushMatrix();
        glTranslatef(x,y,z);
        glRotatef(angle, 0, 0, 1);
        glPushMatrix();
        
        glTranslatef(0, radius, 0);
        
        for(int x = 0; x<sections; x++)
        {
            glPushMatrix();
            glRotatef(rotation*x, 1, 0, 0);
            glTranslatef(0, height, 0);
            glColor3f(1, 0, 0);
            glBegin(GL_QUADS);
            glVertex3f(0, 0, z+width);
            glVertex3f(length, 0, z+width);
            glVertex3f(length, 0, z-width);
            glVertex3f(0, 0, z-width);
            
            glVertex3f(0, 0, z-width);
            glVertex3f(length, 0, z-width);
            glVertex3f(length, 0, z+width);
            glVertex3f(0, 0, z+width);
            glEnd();
            glPopMatrix();
        }
        glPushMatrix();
        glRotatef(rotation/2, 1, 0, 0);
        glBegin(GL_POLYGON);
        for(int x = 0; x<sections; x++)
        {
            glVertex3f(0, cos(toRadians(rotation*-x))*radius, sin(toRadians(rotation*-x))*radius);
        }
        glEnd();
        glPopMatrix();
        glPopMatrix();
        glPopMatrix();
    }
    Cannon(float _x, float _y, float _z, float _sections, float _length, float _radius, float _angle)
    {
        x = _x;
        y = _y;
        z = _z;
        sections = _sections;
        length = _length;
        radius = _radius;
        angle = _angle;
        rotation = 360/sections;
        width = sin(toRadians(rotation/2))*radius;
        height = cos(toRadians(rotation/2))*radius;
        if(sections < 3){
            sections = 3;
        }
    }
private:
    float x, y, z, sections, length, radius, angle, rotation, width, height;
};

Cannon c = Cannon(-50, 0, 0, 8, 30, 5, 45);

class Particle{
public:
    float getX()
    {
        return x;
    }
    float getY()
    {
        return y;
    }
    float getZ()
    {
        return z;
    }
    bool getDeath()
    {
        if(marked && fade<=0)
            return true;
        else
            return false;
    }
    void setTimeOffset(float _offset)
    {
        timeOffset += _offset;
    }
    
    void applyForces()
    {
        f = 0.75;
        
        d  = preciseDiff(current, previous) - timeOffset;
        
        rotationAngle+=rotationSpeed*d;
        
        yspeed+=(gravity*d);
        
        x+=(xspeed*d);
        y+=(yspeed*d);
        z+=(zspeed*d);
        
        if(y<=radius && yspeed<0 && (x<50 && x>-50) && (z<50 && z>-50))
        {
            yspeed = -yspeed*f;
            y = radius;
        }
        timeOffset = 0;
        previous = current;
        lived += d;
        checkLifeTime();
        if(marked && fade>=0)
        {
            fade = 1-(lived-lifespan);
            if(fade<0)
                fade = 0;
        }
    }
    
    void draw()
    {
        colour[0] = red;
        colour[1] = green;
        colour[2] = blue;
        colour[3] = fade;
        
        glPushMatrix();
        glColor4fv(colour);
        glTranslatef(x, y, z);
        glRotatef(rotationAngle, rotationMultipliers[0], rotationMultipliers[1], rotationMultipliers[2]);
        glutSolidSphere(radius, 16, 16);
        glPopMatrix();
        
    }
    Particle()
    {
        previous = current;
        timeOffset = 0;
        d = 0;
        radius = 1.5;
        gravity = -9.8;
        mass = (((float)rand()/RAND_MAX)*(15-10))+10;
        gravity = gravity*mass;
        
        rotationAngle = (((float)rand()/RAND_MAX)*(25-1))+1;
        rotationSpeed = (((float)rand()/RAND_MAX)*(5-1))+1;
        rotationMultipliers[0] = ((float)rand()/RAND_MAX);
        rotationMultipliers[1] = ((float)rand()/RAND_MAX);
        rotationMultipliers[2] = ((float)rand()/RAND_MAX);
        red = ((float)rand()/RAND_MAX);
        green = ((float)rand()/RAND_MAX);
        blue = ((float)rand()/RAND_MAX);
        
        x = 0;
        y = radius;
        z = 0;
        xspeed = (((float)rand()/RAND_MAX)*(5+5))-5;
        yspeed = (((float)rand()/RAND_MAX)*(140-120)+120);
        zspeed = (((float)rand()/RAND_MAX)*(5+5))-5;
        lifespan = 5;

        marked = false;
        fade = 1;
        lived = 0;
    }
    
private:
    void checkLifeTime()
    {
        if(lived > lifespan && !marked)
        {
            marked = true;
        }
    }
    float x, y, z, radius, mass;
    float gravity, xspeed, yspeed, zspeed, rotationAngle, rotationMultipliers[3], rotationSpeed;
    float fade, colour[4], red, green, blue, lifespan, lived, d, f, timeOffset;
    timeval previous;
    bool marked;
};

vector<Particle> parts;

// left
class Particle2{
public:
    float getX()
    {
        return x;
    }
    float getY()
    {
        return y;
    }
    float getZ()
    {
        return z;
    }
    bool getDeath()
    {
        if(marked && fade<=0)
            return true;
        else
            return false;
    }
    void setTimeOffset(float _offset)
    {
        timeOffset += _offset;
    }
    
    void applyForces()
    {
        f = 0.75;
        
        d  = preciseDiff(current, previous) - timeOffset;
        
        rotationAngle+=rotationSpeed*d;
        
        yspeed+=(gravity*d);
        
        x+=(xspeed*d);
        y+=(yspeed*d);
        z+=(zspeed*d);
        
        if(y<=radius && yspeed<0 && (x<50 && x>-50) && (z<50 && z>-50))
        {
            yspeed = -yspeed*f;
            y = radius;
        }
        timeOffset = 0;
        previous = current;
        lived += d;
        checkLifeTime();
        if(marked && fade>=0)
        {
            fade = 1-(lived-lifespan);
            if(fade<0)
                fade = 0;
        }
    }
    
    void draw()
    {
        colour[0] = 0*red;
        colour[1] = 1*green;
        colour[2] = 0*blue;
        colour[3] = 0.8*fade;
        
        glPushMatrix();
        glColor4fv(colour);
        glTranslatef(x, y, z);
        glRotatef(rotationAngle, rotationMultipliers[0], rotationMultipliers[1], rotationMultipliers[2]);
        glutSolidSphere(radius, 16, 16);
        glPopMatrix();
        
    }
    Particle2()
    {
        previous = current;
        timeOffset = 0;
        d = 0;
        radius = 2;
        gravity = -9.8;
        mass = (((float)rand()/RAND_MAX)*(15-10))+10;
        gravity = gravity*mass;
        
        rotationAngle = (((float)rand()/RAND_MAX)*(25-1))+1;
        rotationSpeed = (((float)rand()/RAND_MAX)*(5-1))+1;
        rotationMultipliers[0] = ((float)rand()/RAND_MAX);
        rotationMultipliers[1] = ((float)rand()/RAND_MAX);
        rotationMultipliers[2] = ((float)rand()/RAND_MAX);
        red = ((float)rand()/RAND_MAX);
        green = ((float)rand()/RAND_MAX);
        blue = ((float)rand()/RAND_MAX);
        
        float cannonAngle = toRadians(c.getAngle());
        x = c.getX()+(c.getLength()*(cos(cannonAngle)))-(c.getRadius()*sin(cannonAngle));
        y = c.getY()+(c.getLength()*(sin(cannonAngle)))+(c.getRadius()*cos(cannonAngle));
        z = 0;
        xspeed = cos(cannonAngle)*80;
        yspeed = sin(cannonAngle)*80;
        zspeed = (((float)rand()/RAND_MAX)*(10))-5;

        lifespan = 0.2;
        
        marked = false;
        fade = 1;
        lived = 0;
    }
    
private:
    void checkLifeTime()
    {
        if(lived > lifespan && !marked)
        {
            marked = true;
        }
    }
    float x, y, z, radius, mass;
    float gravity, xspeed, yspeed, zspeed, rotationAngle, rotationMultipliers[3], rotationSpeed;
    float fade, colour[4], red, green, blue, lifespan, lived, d, f, timeOffset;
    timeval previous;
    bool marked;
};

vector<Particle2> parts2;

class Particle3{
public:
    float getX()
    {
        return x;
    }
    float getY()
    {
        return y;
    }
    float getZ()
    {
        return z;
    }
    bool getDeath()
    {
        if(marked && fade<=0)
            return true;
        else
            return false;
    }
    void setTimeOffset(float _offset)
    {
        timeOffset += _offset;
    }
    
    void applyForces()
    {
        f = 0.75;
        
        d  = preciseDiff(current, previous) - timeOffset;
        
        rotationAngle+=rotationSpeed*d;
        
        yspeed+=(gravity*d);
        
        x+=(xspeed*d);
        y+=(yspeed*d);
        z+=(zspeed*d);
        
        if(y<=radius && yspeed<0 && (x<50 && x>-50) && (z<50 && z>-50))
        {
            yspeed = -yspeed*f;
            y = radius;
        }
        timeOffset = 0;
        previous = current;
        lived += d;
        checkLifeTime();
        if(marked && fade>=0)
        {
            fade = 1-(lived-lifespan);
            if(fade<0)
                fade = 0;
        }
    }
    
    void draw()
    {
        colour[0] = 0*red;
        colour[1] = 0*green;
        colour[2] = 1*blue;
        colour[3] = 0.8*fade;
        
        glPushMatrix();
        glColor4fv(colour);
        glTranslatef(x, y, z);
        glRotatef(rotationAngle, rotationMultipliers[0], rotationMultipliers[1], rotationMultipliers[2]);
        glutSolidSphere(radius, 16, 16);
        glPopMatrix();
        
    }
    Particle3()
    {
        previous = current;
        timeOffset = 0;
        d = 0;
        radius = 2;
        gravity = -9.8;
        mass = (((float)rand()/RAND_MAX)*(15-10))+10;
        gravity = gravity*mass;
        
        rotationAngle = (((float)rand()/RAND_MAX)*(25-1))+1;
        rotationSpeed = (((float)rand()/RAND_MAX)*(5-1))+1;
        rotationMultipliers[0] = ((float)rand()/RAND_MAX);
        rotationMultipliers[1] = ((float)rand()/RAND_MAX);
        rotationMultipliers[2] = ((float)rand()/RAND_MAX);
        red = ((float)rand()/RAND_MAX);
        green = ((float)rand()/RAND_MAX);
        blue = ((float)rand()/RAND_MAX);
        
        float cannonAngle = toRadians(c.getAngle());
        x = 60+c.getX()+(c.getLength()*(cos(cannonAngle)))-(c.getRadius()*sin(cannonAngle));
        y = c.getY()+(c.getLength()*(sin(cannonAngle)))+(c.getRadius()*cos(cannonAngle));
        z = 0;
        xspeed = -(cos(cannonAngle)*80);
        yspeed = sin(cannonAngle)*80;
        zspeed = (((float)rand()/RAND_MAX)*(10))-5;
        
        lifespan = 0.2;
        
        marked = false;
        fade = 1;
        lived = 0;
    }
    
private:
    void checkLifeTime()
    {
        if(lived > lifespan && !marked)
        {
            marked = true;
        }
    }
    float x, y, z, radius, mass;
    float gravity, xspeed, yspeed, zspeed, rotationAngle, rotationMultipliers[3], rotationSpeed;
    float fade, colour[4], red, green, blue, lifespan, lived, d, f, timeOffset;
    timeval previous;
    bool marked;
};

vector<Particle3> parts3;

class Particle4{
public:
    float getX()
    {
        return x;
    }
    float getY()
    {
        return y;
    }
    float getZ()
    {
        return z;
    }
    bool getDeath()
    {
        if(marked && fade<=0)
            return true;
        else
            return false;
    }
    void setTimeOffset(float _offset)
    {
        timeOffset += _offset;
    }
    
    void applyForces()
    {
        f = 0.75;
        
        d  = preciseDiff(current, previous) - timeOffset;
        
        rotationAngle+=rotationSpeed*d;
        
        yspeed+=(gravity*d);
        
        x+=(xspeed*d);
        y+=(yspeed*d);
        z+=(zspeed*d);
        
        if(y<=radius && yspeed<0 && (x<50 && x>-50) && (z<50 && z>-50))
        {
            yspeed = -yspeed*f;
            y = radius;
        }
        timeOffset = 0;
        previous = current;
        lived += d;
        checkLifeTime();
        if(marked && fade>=0)
        {
            fade = 1-(lived-lifespan);
            if(fade<0)
                fade = 0;
        }
    }
    
    void draw()
    {
        colour[0] = 0.5*red;
        colour[1] = 0*green;
        colour[2] = 0.5*blue;
        colour[3] = 0.8*fade;
        
        glPushMatrix();
        glColor4fv(colour);
        glTranslatef(x, y, z);
        glRotatef(rotationAngle, rotationMultipliers[0], rotationMultipliers[1], rotationMultipliers[2]);
        glutSolidSphere(radius, 16, 16);
        glPopMatrix();
        
    }
    Particle4()
    {
        previous = current;
        timeOffset = 0;
        d = 0;
        radius = 2;
        gravity = -9.8;
        mass = (((float)rand()/RAND_MAX)*(15-10))+10;
        gravity = gravity*mass;
        
        rotationAngle = (((float)rand()/RAND_MAX)*(25-1))+1;
        rotationSpeed = (((float)rand()/RAND_MAX)*(5-1))+1;
        rotationMultipliers[0] = ((float)rand()/RAND_MAX);
        rotationMultipliers[1] = ((float)rand()/RAND_MAX);
        rotationMultipliers[2] = ((float)rand()/RAND_MAX);
        red = ((float)rand()/RAND_MAX);
        green = ((float)rand()/RAND_MAX);
        blue = ((float)rand()/RAND_MAX);
        
        float cannonAngle = toRadians(c.getAngle());
        x = 47+c.getX()+(c.getLength()*(cos(cannonAngle)))-(c.getRadius()*sin(cannonAngle));
        y = c.getY()+(c.getLength()*(sin(cannonAngle)))+(c.getRadius()*cos(cannonAngle));
        z = 26;
        xspeed = -(cos(cannonAngle)*40);
        yspeed = sin(cannonAngle)*80;
        zspeed = (((float)rand()/RAND_MAX)*(10))-5-40;
        
        lifespan = 0.2;
        
        marked = false;
        fade = 1;
        lived = 0;
    }
    
private:
    void checkLifeTime()
    {
        if(lived > lifespan && !marked)
        {
            marked = true;
        }
    }
    float x, y, z, radius, mass;
    float gravity, xspeed, yspeed, zspeed, rotationAngle, rotationMultipliers[3], rotationSpeed;
    float fade, colour[4], red, green, blue, lifespan, lived, d, f, timeOffset;
    timeval previous;
    bool marked;
};

vector<Particle4> parts4;

class Particle5{
public:
    float getX()
    {
        return x;
    }
    float getY()
    {
        return y;
    }
    float getZ()
    {
        return z;
    }
    bool getDeath()
    {
        if(marked && fade<=0)
            return true;
        else
            return false;
    }
    void setTimeOffset(float _offset)
    {
        timeOffset += _offset;
    }
    
    void applyForces()
    {
        f = 0.75;
        
        d  = preciseDiff(current, previous) - timeOffset;
        
        rotationAngle+=rotationSpeed*d;
        
        yspeed+=(gravity*d);
        
        x+=(xspeed*d);
        y+=(yspeed*d);
        z+=(zspeed*d);
        
        if(y<=radius && yspeed<0 && (x<50 && x>-50) && (z<50 && z>-50))
        {
            yspeed = -yspeed*f;
            y = radius;
        }
        timeOffset = 0;
        previous = current;
        lived += d;
        checkLifeTime();
        if(marked && fade>=0)
        {
            fade = 1-(lived-lifespan);
            if(fade<0)
                fade = 0;
        }
    }
    
    void draw()
    {
        colour[0] = 0.5*red;
        colour[1] = 0.5*green;
        colour[2] = 0*blue;
        colour[3] = 0.8*fade;
        
        glPushMatrix();
        glColor4fv(colour);
        glTranslatef(x, y, z);
        glRotatef(rotationAngle, rotationMultipliers[0], rotationMultipliers[1], rotationMultipliers[2]);
        glutSolidSphere(radius, 16, 16);
        glPopMatrix();
        
    }
    Particle5()
    {
        previous = current;
        timeOffset = 0;
        d = 0;
        radius = 2;
        gravity = -9.8;
        mass = (((float)rand()/RAND_MAX)*(15-10))+10;
        gravity = gravity*mass;
        
        rotationAngle = (((float)rand()/RAND_MAX)*(25-1))+1;
        rotationSpeed = (((float)rand()/RAND_MAX)*(5-1))+1;
        rotationMultipliers[0] = ((float)rand()/RAND_MAX);
        rotationMultipliers[1] = ((float)rand()/RAND_MAX);
        rotationMultipliers[2] = ((float)rand()/RAND_MAX);
        red = ((float)rand()/RAND_MAX);
        green = ((float)rand()/RAND_MAX);
        blue = ((float)rand()/RAND_MAX);
        
        float cannonAngle = toRadians(c.getAngle());
        x = 47+c.getX()+(c.getLength()*(cos(cannonAngle)))-(c.getRadius()*sin(cannonAngle));
        y = c.getY()+(c.getLength()*(sin(cannonAngle)))+(c.getRadius()*cos(cannonAngle));
        z = -26;
        xspeed = -(cos(cannonAngle)*40);
        yspeed = sin(cannonAngle)*80;
        zspeed = -((((float)rand()/RAND_MAX)*(10))-5-40);
        
        lifespan = 0.2;
        
        marked = false;
        fade = 1;
        lived = 0;
    }
    
private:
    void checkLifeTime()
    {
        if(lived > lifespan && !marked)
        {
            marked = true;
        }
    }
    float x, y, z, radius, mass;
    float gravity, xspeed, yspeed, zspeed, rotationAngle, rotationMultipliers[3], rotationSpeed;
    float fade, colour[4], red, green, blue, lifespan, lived, d, f, timeOffset;
    timeval previous;
    bool marked;
};

vector<Particle5> parts5;

class Particle6{
public:
    float getX()
    {
        return x;
    }
    float getY()
    {
        return y;
    }
    float getZ()
    {
        return z;
    }
    bool getDeath()
    {
        if(marked && fade<=0)
            return true;
        else
            return false;
    }
    void setTimeOffset(float _offset)
    {
        timeOffset += _offset;
    }
    
    void applyForces()
    {
        f = 0.75;
        
        d  = preciseDiff(current, previous) - timeOffset;
        
        rotationAngle+=rotationSpeed*d;
        
        yspeed+=(gravity*d);
        
        x+=(xspeed*d);
        y+=(yspeed*d);
        z+=(zspeed*d);
        
        if(y<=radius && yspeed<0 && (x<50 && x>-50) && (z<50 && z>-50))
        {
            yspeed = -yspeed*f;
            y = radius;
        }
        timeOffset = 0;
        previous = current;
        lived += d;
        checkLifeTime();
        if(marked && fade>=0)
        {
            fade = 1-(lived-lifespan);
            if(fade<0)
                fade = 0;
        }
    }
    
    void draw()
    {
        colour[0] = 0*red;
        colour[1] = 0.5*green;
        colour[2] = 0.5*blue;
        colour[3] = 0.8*fade;
        
        glPushMatrix();
        glColor4fv(colour);
        glTranslatef(x, y, z);
        glRotatef(rotationAngle, rotationMultipliers[0], rotationMultipliers[1], rotationMultipliers[2]);
        glutSolidSphere(radius, 16, 16);
        glPopMatrix();
        
    }
    Particle6()
    {
        previous = current;
        timeOffset = 0;
        d = 0;
        radius = 2;
        gravity = -9.8;
        mass = (((float)rand()/RAND_MAX)*(15-10))+10;
        gravity = gravity*mass;
        
        rotationAngle = (((float)rand()/RAND_MAX)*(25-1))+1;
        rotationSpeed = (((float)rand()/RAND_MAX)*(5-1))+1;
        rotationMultipliers[0] = ((float)rand()/RAND_MAX);
        rotationMultipliers[1] = ((float)rand()/RAND_MAX);
        rotationMultipliers[2] = ((float)rand()/RAND_MAX);
        red = ((float)rand()/RAND_MAX);
        green = ((float)rand()/RAND_MAX);
        blue = ((float)rand()/RAND_MAX);
        
        float cannonAngle = toRadians(c.getAngle());
        x = 16+c.getX()+(c.getLength()*(cos(cannonAngle)))-(c.getRadius()*sin(cannonAngle));
        y = c.getY()+(c.getLength()*(sin(cannonAngle)))+(c.getRadius()*cos(cannonAngle));
        z = -26;
        xspeed = cos(cannonAngle)*40;
        yspeed = sin(cannonAngle)*80;
        zspeed = -((((float)rand()/RAND_MAX)*(10))-5-40);
        
        lifespan = 0.2;
        
        marked = false;
        fade = 1;
        lived = 0;
    }
    
private:
    void checkLifeTime()
    {
        if(lived > lifespan && !marked)
        {
            marked = true;
        }
    }
    float x, y, z, radius, mass;
    float gravity, xspeed, yspeed, zspeed, rotationAngle, rotationMultipliers[3], rotationSpeed;
    float fade, colour[4], red, green, blue, lifespan, lived, d, f, timeOffset;
    timeval previous;
    bool marked;
};

vector<Particle6> parts6;

class Particle7{
public:
    float getX()
    {
        return x;
    }
    float getY()
    {
        return y;
    }
    float getZ()
    {
        return z;
    }
    bool getDeath()
    {
        if(marked && fade<=0)
            return true;
        else
            return false;
    }
    void setTimeOffset(float _offset)
    {
        timeOffset += _offset;
    }
    
    void applyForces()
    {
        f = 0.75;
        
        d  = preciseDiff(current, previous) - timeOffset;
        
        rotationAngle+=rotationSpeed*d;
        
        yspeed+=(gravity*d);
        
        x+=(xspeed*d);
        y+=(yspeed*d);
        z+=(zspeed*d);
        
        if(y<=radius && yspeed<0 && (x<50 && x>-50) && (z<50 && z>-50))
        {
            yspeed = -yspeed*f;
            y = radius;
        }
        timeOffset = 0;
        previous = current;
        lived += d;
        checkLifeTime();
        if(marked && fade>=0)
        {
            fade = 1-(lived-lifespan);
            if(fade<0)
                fade = 0;
        }
    }
    
    void draw()
    {
        colour[0] = 0.2*red;
        colour[1] = 0.4*green;
        colour[2] = 0.7*blue;
        colour[3] = 0.8*fade;
        
        glPushMatrix();
        glColor4fv(colour);
        glTranslatef(x, y, z);
        glRotatef(rotationAngle, rotationMultipliers[0], rotationMultipliers[1], rotationMultipliers[2]);
        glutSolidSphere(radius, 16, 16);
        glPopMatrix();
        
    }
    Particle7()
    {
        previous = current;
        timeOffset = 0;
        d = 0;
        radius = 2;
        gravity = -9.8;
        mass = (((float)rand()/RAND_MAX)*(15-10))+10;
        gravity = gravity*mass;
        
        rotationAngle = (((float)rand()/RAND_MAX)*(25-1))+1;
        rotationSpeed = (((float)rand()/RAND_MAX)*(5-1))+1;
        rotationMultipliers[0] = ((float)rand()/RAND_MAX);
        rotationMultipliers[1] = ((float)rand()/RAND_MAX);
        rotationMultipliers[2] = ((float)rand()/RAND_MAX);
        red = ((float)rand()/RAND_MAX);
        green = ((float)rand()/RAND_MAX);
        blue = ((float)rand()/RAND_MAX);
        
        float cannonAngle = toRadians(c.getAngle());
        x = 16+c.getX()+(c.getLength()*(cos(cannonAngle)))-(c.getRadius()*sin(cannonAngle));
        y = c.getY()+(c.getLength()*(sin(cannonAngle)))+(c.getRadius()*cos(cannonAngle));
        z = 26;
        xspeed = cos(cannonAngle)*40;
        yspeed = sin(cannonAngle)*80;
        zspeed = (((float)rand()/RAND_MAX)*(10))-5-40;
        
        lifespan = 0.2;
        
        marked = false;
        fade = 1;
        lived = 0;
    }
    
private:
    void checkLifeTime()
    {
        if(lived > lifespan && !marked)
        {
            marked = true;
        }
    }
    float x, y, z, radius, mass;
    float gravity, xspeed, yspeed, zspeed, rotationAngle, rotationMultipliers[3], rotationSpeed;
    float fade, colour[4], red, green, blue, lifespan, lived, d, f, timeOffset;
    timeval previous;
    bool marked;
};

vector<Particle7> parts7;


// rain
class Particle8{
public:
    float getX()
    {
        return x;
    }
    float getY()
    {
        return y;
    }
    float getZ()
    {
        return z;
    }
    bool getDeath()
    {
        if(marked && fade<=0)
            return true;
        else
            return false;
    }
    void setTimeOffset(float _offset)
    {
        timeOffset += _offset;
    }
    
    void applyForces()
    {
        f = 0.75;
        
        d  = preciseDiff(current, previous) - timeOffset;
        
        rotationAngle+=rotationSpeed*d;
        
        yspeed+=(gravity*d);
        
        x+=(xspeed*d);
        y+=(yspeed*d);
        z+=(zspeed*d);
        
        if(y<=radius && yspeed<0 && (x<50 && x>-50) && (z<50 && z>-50))
        {
            yspeed = -yspeed*f;
            y = radius;
        }
        timeOffset = 0;
        previous = current;
        lived += d;
        checkLifeTime();
        if(marked && fade>=0)
        {
            fade = 1-(lived-lifespan);
            if(fade<0)
                fade = 0;
        }
    }
    
    void draw()
    {
        colour[0] = 0;
        colour[1] = 0.5;
        colour[2] = 0.5;
        colour[3] = fade;
        
        glPushMatrix();
        glColor4fv(colour);
        glTranslatef(x, y, z);
        glRotatef(rotationAngle, rotationMultipliers[0], rotationMultipliers[1], rotationMultipliers[2]);
        glutSolidSphere(radius, 16, 16);
        glPopMatrix();
        
    }
    Particle8()
    {
        previous = current;
        timeOffset = 0;
        d = 0;
        radius = 0.8;
        gravity = -9.8;
        mass = (((float)rand()/RAND_MAX)*(15-10))+10;
        gravity = gravity*mass;
        
        rotationAngle = (((float)rand()/RAND_MAX)*(25-1))+1;
        rotationSpeed = (((float)rand()/RAND_MAX)*(5-1))+1;
        rotationMultipliers[0] = ((float)rand()/RAND_MAX);
        rotationMultipliers[1] = ((float)rand()/RAND_MAX);
        rotationMultipliers[2] = ((float)rand()/RAND_MAX);
        red = ((float)rand()/RAND_MAX);
        green = ((float)rand()/RAND_MAX);
        blue = ((float)rand()/RAND_MAX);
        
        x = (((float)rand()/RAND_MAX)*(50+50))-50;
        y = 120;
        z = (((float)rand()/RAND_MAX)*(50+50))-50;
        xspeed = 0;
        yspeed = 0;
        zspeed = 0;
        lifespan = 5;
        
        marked = false;
        fade = 1;
        lived = 0;
    }
    
private:
    void checkLifeTime()
    {
        if(lived > lifespan && !marked)
        {
            marked = true;
        }
    }
    float x, y, z, radius, mass;
    float gravity, xspeed, yspeed, zspeed, rotationAngle, rotationMultipliers[3], rotationSpeed;
    float fade, colour[4], red, green, blue, lifespan, lived, d, f, timeOffset;
    timeval previous;
    bool marked;
};

vector<Particle8> parts8;

// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE        0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// size of the box:

const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// minimum allowable scale factor:

const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


// which projection:

enum Projections
{
    ORTHO,
    PERSP
};


// which button:

enum ButtonVals
{
    RESET,
    QUIT
};


// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };


// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };


// the color numbers:
// this order must match the radio button order

enum Colors
{
    RED,
    YELLOW,
    GREEN,
    CYAN,
    BLUE,
    MAGENTA,
    WHITE,
    BLACK
};

char * ColorNames[ ] =
{
    "Red",
    "Yellow",
    "Green",
    "Cyan",
    "Blue",
    "Magenta",
    "White",
    "Black"
};


// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] =
{
    { 1., 0., 0. },        // red
    { 1., 1., 0. },        // yellow
    { 0., 1., 0. },        // green
    { 0., 1., 1. },        // cyan
    { 0., 0., 1. },        // blue
    { 1., 0., 1. },        // magenta
    { 1., 1., 1. },        // white
    { 0., 0., 0. },        // black
};


// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };


// non-constant global variables:

int        ActiveButton;            // current button that is down
GLuint     AxesList;                // list to hold the axes
int        AxesOn;                    // != 0 means to draw the axes
int        DebugOn;                // != 0 means to print debugging info
int        DepthCueOn;                // != 0 means to use intensity depth cueing
GLuint     EarthList;                // object display list
int        MainWindow;                // window id for main graphics window
int        WhichColor;                // index into Colors[ ]
int        WhichProjection;        // ORTHO or PERSP
int        Xmouse, Ymouse;            // mouse values
float      Xrot, Yrot;                // rotation angles in degrees
float      Scale;                    // scaling factor
bool IfDistort;

//
#define MS_PER_CYCLE 3500
GLuint WorldTex;
float Gray[] = {0.5, 0.5, 0.5, 1.};
float White[] = {1., 1., 1., 1.};
float Time;
bool Light0On = 1;
bool Light1On = 1;
bool Light2On = 1;
bool Freeze = 0;
GLuint tex0;


struct point {
    float x, y, z;        // coordinates
    float nx, ny, nz;    // surface normal
    float s, t;        // texture coords
};

int NumLngs, NumLats;
struct point *    Pts;

struct point * PtsPointer(int lat, int lng)
{
    if (lat < 0)    lat += (NumLats - 1);
    if (lng < 0)    lng += (NumLngs - 1);
    if (lat > NumLats - 1)    lat -= (NumLats - 1);
    if (lng > NumLngs - 1)    lng -= (NumLngs - 1);
    return &Pts[NumLngs*lat + lng];
}

// function prototypes:

void    Animate( );
void    Display( );
void    DoAxesMenu( int );
void    DoColorMenu( int );
void    DoDepthMenu( int );
void    DoDebugMenu( int );
void    DoMainMenu( int );
void    DoProjectMenu( int );
void    DoRasterString( float, float, float, char * );
void    DoStrokeString( float, float, float, float, char * );
float   ElapsedSeconds( );
void    InitGraphics( );
void    InitLists( );
void    InitMenus( );
void    Keyboard( unsigned char, int, int );
void    MouseButton( int, int, int, int );
void    MouseMotion( int, int );
void    Reset( );
void    Resize( int, int );
void    Visibility( int );
int width = 1024;
int height = 512;

void    Axes( float );
void    HsvRgb( float[3], float [3] );

void DrawPoint(struct point *p)
{
    glNormal3f(p->nx, p->ny, p->nz);
    glTexCoord2f(p->s, p->t);
    glVertex3f(p->x, p->y, p->z);
}

// utility to create an array from 3 separate values:
float *
Array3(float a, float b, float c)
{
    static float array[4];
    array[0] = a;
    array[1] = b;
    array[2] = c;
    array[3] = 1.;
    return array;
}
// utility to create an array from a multiplier and an array:
float *
MulArray3(float factor, float array0[3])
{
    static float array[4];
    array[0] = factor * array0[0];
    array[1] = factor * array0[1];
    array[2] = factor * array0[2];
    array[3] = 1.;
    return array;
}

void
SetMaterial(float r, float g, float b, float shininess)
{
    glMaterialfv(GL_BACK, GL_EMISSION, Array3(0., 0., 0.));
    glMaterialfv(GL_BACK, GL_AMBIENT, MulArray3(.4f, Gray));
    glMaterialfv(GL_BACK, GL_DIFFUSE, MulArray3(1., Gray));
    glMaterialfv(GL_BACK, GL_SPECULAR, Array3(0., 0., 0.));
    glMaterialf(GL_BACK, GL_SHININESS, 2.f);
    
    glMaterialfv(GL_FRONT, GL_EMISSION, Array3(0., 0., 0.));
    glMaterialfv(GL_FRONT, GL_AMBIENT, Array3(r, g, b));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, Array3(r, g, b));
    glMaterialfv(GL_FRONT, GL_SPECULAR, MulArray3(.8f, White));
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void
SetPointLight(int ilight, float x, float y, float z, float r, float g, float b)
{
    glLightfv(ilight, GL_POSITION, Array3(x, y, z));
    glLightfv(ilight, GL_AMBIENT, Array3(0., 0., 0.));
    glLightfv(ilight, GL_DIFFUSE, Array3(r, g, b));
    glLightfv(ilight, GL_SPECULAR, Array3(r, g, b));
    glLightf(ilight, GL_CONSTANT_ATTENUATION, 1.);
    glLightf(ilight, GL_LINEAR_ATTENUATION, 0.);
    glLightf(ilight, GL_QUADRATIC_ATTENUATION, 0.);
    glEnable(ilight);
}

void
SetSpotLight(int ilight, float x, float y, float z, float xdir, float ydir, float zdir, float r, float g, float b)
{
    glLightfv(ilight, GL_POSITION, Array3(x, y, z));
    glLightfv(ilight, GL_SPOT_DIRECTION, Array3(xdir, ydir, zdir));
    glLightf(ilight, GL_SPOT_EXPONENT, 1);
    glLightf(ilight, GL_SPOT_CUTOFF, 45.);
    glLightfv(ilight, GL_AMBIENT, Array3(0., 0., 0.));
    glLightfv(ilight, GL_DIFFUSE, Array3(r, g, b));
    glLightfv(ilight, GL_SPECULAR, Array3(r, g, b));
    glLightf(ilight, GL_CONSTANT_ATTENUATION, 1.);
    glLightf(ilight, GL_LINEAR_ATTENUATION, 0.);
    glLightf(ilight, GL_QUADRATIC_ATTENUATION, 0.);
    glEnable(ilight);
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate( )
{
    // put animation stuff in here -- change some global variables
    // for Display( ) to find:
    
    int ms = glutGet(GLUT_ELAPSED_TIME);
    ms %= MS_PER_CYCLE;
    Time = (float)ms / (float)(MS_PER_CYCLE - 1);
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
    if( DebugOn != 0 )
    {
        fprintf( stderr, "Display\n" );
    }


    // set which window we want to do the graphics into:

    glutSetWindow( MainWindow );


    // erase the background:

    glDrawBuffer( GL_BACK );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glEnable( GL_DEPTH_TEST );


    // specify shading to be flat:

    glShadeModel( GL_SMOOTH );
    
    
    // set the viewport to a square centered in the window:
    
    GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
    GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
    GLsizei v = vx < vy ? vx : vy;            // minimum dimension
    GLint xl = ( vx - v ) / 2;
    GLint yb = ( vy - v ) / 2;
    glViewport( xl, yb,  v, v );
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40, 1, 0.1, 1000);

    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );

    // set the eye position, look-at position, and up-vector:

    gluLookAt( 150, 190, 150,     0., 0., 0.,     0., 1., 0. );

    
    // rotate the scene:
    
    glRotatef( (GLfloat)Yrot, 0., 1., 0. );
    glRotatef( (GLfloat)Xrot, 1., 0., 0. );

    // uniformly scale the scene:
    
    if( Scale < MINSCALE )
        Scale = MINSCALE;
    glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

    // set the fog parameters:

    if( DepthCueOn != 0 )
    {
        glFogi( GL_FOG_MODE, FOGMODE );
        glFogfv( GL_FOG_COLOR, FOGCOLOR );
        glFogf( GL_FOG_DENSITY, FOGDENSITY );
        glFogf( GL_FOG_START, FOGSTART );
        glFogf( GL_FOG_END, FOGEND );
        glEnable( GL_FOG );
    }
    else
    {
        glDisable( GL_FOG );
    }
    
    if(paused && !frozen)
    {
        gettimeofday(&tfrozen, NULL);
        frozen = true;
    }
    else if(!paused && frozen)
    {
        gettimeofday(&thawed, NULL);
        pauseOffset = preciseDiff(thawed, tfrozen);
        for(int x = 0; x<parts.size(); x++)
        {
            parts[x].setTimeOffset(pauseOffset);
        }
        for(int x = 0; x<parts2.size(); x++)
        {
            parts2[x].setTimeOffset(pauseOffset);
        }
        for(int x = 0; x<parts3.size(); x++)
        {
            parts3[x].setTimeOffset(pauseOffset);
        }
        for(int x = 0; x<parts4.size(); x++)
        {
            parts4[x].setTimeOffset(pauseOffset);
        }
        for(int x = 0; x<parts5.size(); x++)
        {
            parts5[x].setTimeOffset(pauseOffset);
        }
        for(int x = 0; x<parts6.size(); x++)
        {
            parts6[x].setTimeOffset(pauseOffset);
        }
        for(int x = 0; x<parts7.size(); x++)
        {
            parts7[x].setTimeOffset(pauseOffset);
        }
        for(int x = 0; x<parts8.size(); x++)
        {
            parts8[x].setTimeOffset(pauseOffset);
        }

        
        
        frozen = false;
    }
    
    gettimeofday(&current, NULL);
    
    pTime = preciseDiff(current, point) - pauseOffset;
    
    if(pTime > tspacing && !paused)
    {
        parts.push_back(Particle());
        parts2.push_back(Particle2());
        parts3.push_back(Particle3());
        parts4.push_back(Particle4());
        parts5.push_back(Particle5());
        parts6.push_back(Particle6());
        parts7.push_back(Particle7());
        parts8.push_back(Particle8());
        
        gettimeofday(&point, NULL);
    }
    
    if(!paused)
    {
        for(int x = 0; x<parts.size(); x++)
        {
            parts[x].applyForces();
        }
        for(int x = 0; x<parts2.size(); x++)
        {
            parts2[x].applyForces();
        }
        for(int x = 0; x<parts3.size(); x++)
        {
            parts3[x].applyForces();
        }
        for(int x = 0; x<parts4.size(); x++)
        {
            parts4[x].applyForces();
        }
        for(int x = 0; x<parts5.size(); x++)
        {
            parts5[x].applyForces();
        }
        for(int x = 0; x<parts6.size(); x++)
        {
            parts6[x].applyForces();
        }
        for(int x = 0; x<parts7.size(); x++)
        {
            parts7[x].applyForces();
        }
        for(int x = 0; x<parts8.size(); x++)
        {
            parts8[x].applyForces();
        }


    }
    float l = -1;
    
    
    for(int x = 0; x<parts.size(); x++)
    {
        if(parts[x].getDeath())
        {
            l = x;
        }
        else if(l != -1)
        {
            if(p<=l)
                cstate = out;
            else
                p-=l;
            for(int z = 0; z<l; z++)
            {
                parts.erase(parts.begin());
            }
            break;
        }
    }
    for(int x = 0; x<parts2.size(); x++)
    {
        if(parts2[x].getDeath())
        {
            l = x;
        }
        else if(l != -1)
        {
            if(p<=l)
                cstate = out;
            else
                p-=l;
            for(int z = 0; z<l; z++)
            {
                parts2.erase(parts2.begin());
            }
            break;
        }
    }
    for(int x = 0; x<parts3.size(); x++)
    {
        if(parts3[x].getDeath())
        {
            l = x;
        }
        else if(l != -1)
        {
            if(p<=l)
                cstate = out;
            else
                p-=l;
            for(int z = 0; z<l; z++)
            {
                parts3.erase(parts3.begin());
            }
            break;
        }
    }
    for(int x = 0; x<parts4.size(); x++)
    {
        if(parts4[x].getDeath())
        {
            l = x;
        }
        else if(l != -1)
        {
            if(p<=l)
                cstate = out;
            else
                p-=l;
            for(int z = 0; z<l; z++)
            {
                parts4.erase(parts4.begin());
            }
            break;
        }
    }
    for(int x = 0; x<parts5.size(); x++)
    {
        if(parts5[x].getDeath())
        {
            l = x;
        }
        else if(l != -1)
        {
            if(p<=l)
                cstate = out;
            else
                p-=l;
            for(int z = 0; z<l; z++)
            {
                parts5.erase(parts5.begin());
            }
            break;
        }
    }
    for(int x = 0; x<parts6.size(); x++)
    {
        if(parts6[x].getDeath())
        {
            l = x;
        }
        else if(l != -1)
        {
            if(p<=l)
                cstate = out;
            else
                p-=l;
            for(int z = 0; z<l; z++)
            {
                parts6.erase(parts6.begin());
            }
            break;
        }
    }
    for(int x = 0; x<parts7.size(); x++)
    {
        if(parts7[x].getDeath())
        {
            l = x;
        }
        else if(l != -1)
        {
            if(p<=l)
                cstate = out;
            else
                p-=l;
            for(int z = 0; z<l; z++)
            {
                parts7.erase(parts7.begin());
            }
            break;
        }
    }
    for(int x = 0; x<parts8.size(); x++)
    {
        if(parts8[x].getDeath())
        {
            l = x;
        }
        else if(l != -1)
        {
            if(p<=l)
                cstate = out;
            else
                p-=l;
            for(int z = 0; z<l; z++)
            {
                parts8.erase(parts8.begin());
            }
            break;
        }
    }

    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //Base
    glPushMatrix();
    glShadeModel(GL_SMOOTH);
    glScaled(1, 1, 1);
    glColor3f(1., 1., 1.);
    glTranslated(0, -5, 0);
    glScaled(2, 2, 2);
    glRotated(90, 1, 0, 0);
    SetMaterial(0.5, 0.5, 0.5, 50.);
    glutSolidTorus(10, 25, 5, 6);
    glPopMatrix();
    
    glPushMatrix();
    glShadeModel(GL_SMOOTH);
    glScaled(1, 0.8, 1);
    glColor3f(1., 0., 0.);
    glTranslated(0, -5, 0);
    glScaled(2, 2, 2);
    glRotated(90, 1, 0, 0);
    SetMaterial(0.5, 0.5, 0.5, 50.);
    glutSolidTorus(11, 25, 5, 6);
    glPopMatrix();
    
    glPushMatrix();
    glShadeModel(GL_SMOOTH);
    glTranslated(0, -3, 0);
    glScaled(1, 0.8, 1);
    glColor3f(1., 1., 0.);
    glTranslated(0, -5, 0);
    glScaled(2, 2, 2);
    glRotated(90, 1, 0, 0);
    SetMaterial(0.5, 0.5, 0.5, 50.);
    glutSolidTorus(12, 25, 5, 6);
    glPopMatrix();
    
    glPushMatrix();
    glShadeModel(GL_SMOOTH);
    glTranslated(0, -3, 0);
    glScaled(1, 0.6, 1);
    glColor3f(0., 1., 1.);
    glTranslated(0, -5, 0);
    glScaled(2, 2, 2);
    glRotated(90, 1, 0, 0);
    SetMaterial(0.5, 0.5, 0.5, 50.);
    glutSolidTorus(13, 25, 5, 6);
    glPopMatrix();
    
    glPushMatrix();
    glShadeModel(GL_SMOOTH);
    glTranslated(0, -3, 0);
    glScaled(1, 0.4, 1);
    glColor3f(1., 0., 1.);
    glTranslated(0, -5, 0);
    glScaled(2, 2, 2);
    glRotated(30, 0, 1, 0);
    glRotated(90, 1, 0, 0);
    SetMaterial(0.5, 0.5, 0.5, 50.);
    glutSolidTorus(14, 25, 5, 6);
    glPopMatrix();
    
    // mid particles
    for(int x = 0; x<parts.size(); x++)
    {
        parts[x].draw();
    }
    for(int x = 0; x<parts2.size(); x++)
    {
        parts2[x].draw();
    }
    for(int x = 0; x<parts3.size(); x++)
    {
        parts3[x].draw();
    }
    for(int x = 0; x<parts4.size(); x++)
    {
        parts4[x].draw();
    }
    for(int x = 0; x<parts5.size(); x++)
    {
        parts5[x].draw();
    }
    for(int x = 0; x<parts6.size(); x++)
    {
        parts6[x].draw();
    }
    for(int x = 0; x<parts7.size(); x++)
    {
        parts7[x].draw();
    }
    for(int x = 0; x<parts8.size(); x++)
    {
        parts8[x].draw();
    }
    
    // draw canons
    c.draw();
    glRotated(60, 0, 1, 0);
    c.draw();
    glRotated(60, 0, 1, 0);
    c.draw();
    glRotated(60, 0, 1, 0);
    c.draw();
    glRotated(60, 0, 1, 0);
    c.draw();
    glRotated(60, 0, 1, 0);
    c.draw();
    
    pauseOffset = 0;
    
    glDisable( GL_DEPTH_TEST );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    gluOrtho2D( 0., 100.,     0., 100. );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    glDisable(GL_LIGHTING);
    glColor3f( 1., 1., 1. );
    DoRasterString( 5., 5., 0., "Final Project" );
    
    glFlush();
    glutSwapBuffers();
    gettimeofday(&instant, NULL);
    glutPostRedisplay();
}

void
DoAxesMenu( int id )
{
    AxesOn = id;
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
    WhichColor = id - RED;
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
    DebugOn = id;
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
    DepthCueOn = id;
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
    switch( id )
    {
        case RESET:
            Reset( );
            break;
            
        case QUIT:
            // gracefully close out the graphics:
            // gracefully close the graphics window:
            // gracefully exit the program:
            glutSetWindow( MainWindow );
            glFinish( );
            glutDestroyWindow( MainWindow );
            exit( 0 );
            break;
            
        default:
            fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
    }
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
    WhichProjection = id;
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
    glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );
    
    char c;            // one character to print
    for( ; ( c = *s ) != '\0'; s++ )
    {
        glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
    }
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
    glPushMatrix( );
    glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
    float sf = ht / ( 119.05f + 33.33f );
    glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
    char c;            // one character to print
    for( ; ( c = *s ) != '\0'; s++ )
    {
        glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
    }
    glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
    // get # of milliseconds since the start of the program:
    
    int ms = glutGet( GLUT_ELAPSED_TIME );
    
    // convert it to seconds:
    
    return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
    glutSetWindow( MainWindow );
    
    int numColors = sizeof( Colors ) / ( 3*sizeof(int) );
    int colormenu = glutCreateMenu( DoColorMenu );
    for( int i = 0; i < numColors; i++ )
    {
        glutAddMenuEntry( ColorNames[i], i );
    }
    
    int axesmenu = glutCreateMenu( DoAxesMenu );
    glutAddMenuEntry( "Off",  0 );
    glutAddMenuEntry( "On",   1 );
    
    int depthcuemenu = glutCreateMenu( DoDepthMenu );
    glutAddMenuEntry( "Off",  0 );
    glutAddMenuEntry( "On",   1 );
    
    int debugmenu = glutCreateMenu( DoDebugMenu );
    glutAddMenuEntry( "Off",  0 );
    glutAddMenuEntry( "On",   1 );
    
    int projmenu = glutCreateMenu( DoProjectMenu );
    glutAddMenuEntry( "Orthographic",  ORTHO );
    glutAddMenuEntry( "Perspective",   PERSP );
    
    int mainmenu = glutCreateMenu( DoMainMenu );
    glutAddSubMenu(   "Axes",          axesmenu);
    glutAddSubMenu(   "Colors",        colormenu);
    glutAddSubMenu(   "Depth Cue",     depthcuemenu);
    glutAddSubMenu(   "Projection",    projmenu );
    glutAddMenuEntry( "Reset",         RESET );
    glutAddSubMenu(   "Debug",         debugmenu);
    glutAddMenuEntry( "Quit",          QUIT );
    
    // attach the pop-up menu to the right mouse button:
    
    glutAttachMenu( GLUT_RIGHT_BUTTON );
}

// initialize the glut and OpenGL libraries:
//    also setup display lists and callback functions

void
InitGraphics( )
{
    unsigned char *Texture;
    int Width, Height;
    // request the display modes:
    // ask for red-green-blue-alpha color, double-buffering, and z-buffering:
    
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    
    // set the initial window configuration:
    
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );
    
    // open the window and set its title:
    
    MainWindow = glutCreateWindow( WINDOWTITLE );
    glutSetWindowTitle( WINDOWTITLE );
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gettimeofday(&current, NULL);
    gettimeofday(&point, NULL);
    gettimeofday(&instant, NULL);
    
    // set the framebuffer clear values:
    glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );
    glutSetWindow( MainWindow );
    glutDisplayFunc( Display );
    glutReshapeFunc( Resize );
    glutKeyboardFunc( Keyboard );
    glutMouseFunc( MouseButton );
    glutMotionFunc( MouseMotion );
    glutPassiveMotionFunc( NULL );
    glutVisibilityFunc( Visibility );
    glutEntryFunc( NULL );
    glutSpecialFunc( NULL );
    glutSpaceballMotionFunc( NULL );
    glutSpaceballRotateFunc( NULL );
    glutSpaceballButtonFunc( NULL );
    glutButtonBoxFunc( NULL );
    glutDialsFunc( NULL );
    glutTabletMotionFunc( NULL );
    glutTabletButtonFunc( NULL );
    glutMenuStateFunc( NULL );
    glutTimerFunc( -1, NULL, 0 );
    glutIdleFunc( Animate );
    
    // init glew (a window must be open to do this):
#ifdef WIN32
    GLenum err = glewInit( );
    if( err != GLEW_OK )
    {
        fprintf( stderr, "glewInit Error\n" );
    }
    else
        fprintf( stderr, "GLEW initialized OK\n" );
    fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif
    
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
    float dx = BOXSIZE / 2.f;
    float dy = BOXSIZE / 2.f;
    float dz = BOXSIZE / 2.f;
    glutSetWindow( MainWindow );
    
    // create the axes:
    
//    AxesList = glGenLists( 1 );
//    glNewList( AxesList, GL_COMPILE );
//    glDisable(GL_LIGHTING);
//    glLineWidth( AXES_WIDTH );
//    Axes( 3 );
//    glLineWidth( 1. );
//    glEnable(GL_LIGHTING);
//    glEndList( );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
    if( DebugOn != 0 )
        fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );
    
    switch( c )
    {
        case 'r':
        case 'R':
            parts.clear();
            parts2.clear();
            parts3.clear();
            parts4.clear();
            parts5.clear();
            parts6.clear();
            parts7.clear();
            parts8.clear();
            
        case 'q':
        case 'Q':
        case ESCAPE:
            DoMainMenu( QUIT );    // will not return here
            break;                // happy compiler
            
        default:
            fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
    }
    
    // force a call to Display( ):
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
    int b = 0;            // LEFT, MIDDLE, or RIGHT
    
    if( DebugOn != 0 )
        fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );
    
    
    // get the proper button bit mask:
    
    switch( button )
    {
        case GLUT_LEFT_BUTTON:
            b = LEFT;        break;
            
        case GLUT_MIDDLE_BUTTON:
            b = MIDDLE;        break;
            
        case GLUT_RIGHT_BUTTON:
            b = RIGHT;        break;
            
        default:
            b = 0;
            fprintf( stderr, "Unknown mouse button: %d\n", button );
    }
    
    
    // button down sets the bit, up clears the bit:
    
    if( state == GLUT_DOWN )
    {
        Xmouse = x;
        Ymouse = y;
        ActiveButton |= b;        // set the proper bit
    }
    else
    {
        ActiveButton &= ~b;        // clear the proper bit
    }
}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
    if( DebugOn != 0 )
        fprintf( stderr, "MouseMotion: %d, %d\n", x, y );
    
    
    int dx = x - Xmouse;        // change in mouse coords
    int dy = y - Ymouse;
    
    if( ( ActiveButton & LEFT ) != 0 )
    {
        Xrot += ( ANGFACT*dy );
        Yrot += ( ANGFACT*dx );
    }
    
    
    if( ( ActiveButton & MIDDLE ) != 0 )
    {
        Scale += SCLFACT * (float) ( dx - dy );
        
        // keep object from turning inside-out or disappearing:
        
        if( Scale < MINSCALE )
            Scale = MINSCALE;
    }
    
    Xmouse = x;            // new current position
    Ymouse = y;
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
    ActiveButton = 0;
    AxesOn = 1;
    DebugOn = 0;
    DepthCueOn = 0;
    Scale  = 1.0;
    WhichColor = WHITE;
    WhichProjection = PERSP;
    Xrot = Yrot = 0.;
    Freeze = 1;
    Light0On = 1;
    Light1On = 1;
    Light2On = 1;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
    if( DebugOn != 0 )
        fprintf( stderr, "ReSize: %d, %d\n", width, height );
    
    // don't really need to do anything since window size is
    // checked each time in Display( ):
    
    glutSetWindow( MainWindow );
    glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
    if( DebugOn != 0 )
        fprintf( stderr, "Visibility: %d\n", state );
    
    if( state == GLUT_VISIBLE )
    {
        glutSetWindow( MainWindow );
        glutPostRedisplay( );
    }
    else
    {
        // could optimize by keeping track of the fact
        // that the window is not visible and avoid
        // animating or redrawing it ...
    }
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
    0.f, 1.f, 0.f, 1.f
};

static float xy[ ] = {
    -.5f, .5f, .5f, -.5f
};

static int xorder[ ] = {
    1, 2, -3, 4
};

static float yx[ ] = {
    0.f, 0.f, -.5f, .5f
};

static float yy[ ] = {
    0.f, .6f, 1.f, 1.f
};

static int yorder[ ] = {
    1, 2, 3, -2, 4
};

static float zx[ ] = {
    1.f, 0.f, 1.f, 0.f, .25f, .75f
};

static float zy[ ] = {
    .5f, .5f, -.5f, -.5f, 0.f, 0.f
};

static int zorder[ ] = {
    1, 2, 3, 4, -5, 6
};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//    Draw a set of 3D axes:
//    (length is the axis length in world coordinates)

void
Axes( float length )
{
    glBegin( GL_LINE_STRIP );
    glVertex3f( length, 0., 0. );
    glVertex3f( 0., 0., 0. );
    glVertex3f( 0., length, 0. );
    glEnd( );
    glBegin( GL_LINE_STRIP );
    glVertex3f( 0., 0., 0. );
    glVertex3f( 0., 0., length );
    glEnd( );
    
    float fact = LENFRAC * length;
    float base = BASEFRAC * length;
    
    glBegin( GL_LINE_STRIP );
    for( int i = 0; i < 4; i++ )
    {
        int j = xorder[i];
        if( j < 0 )
        {
            
            glEnd( );
            glBegin( GL_LINE_STRIP );
            j = -j;
        }
        j--;
        glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
    }
    glEnd( );
    
    glBegin( GL_LINE_STRIP );
    for( int i = 0; i < 5; i++ )
    {
        int j = yorder[i];
        if( j < 0 )
        {
            
            glEnd( );
            glBegin( GL_LINE_STRIP );
            j = -j;
        }
        j--;
        glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
    }
    glEnd( );
    
    glBegin( GL_LINE_STRIP );
    for( int i = 0; i < 6; i++ )
    {
        int j = zorder[i];
        if( j < 0 )
        {
            
            glEnd( );
            glBegin( GL_LINE_STRIP );
            j = -j;
        }
        j--;
        glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
    }
    glEnd( );
    
}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//        glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
    // guarantee valid input:
    
    float h = hsv[0] / 60.f;
    while( h >= 6. )    h -= 6.;
    while( h <  0. )     h += 6.;
    
    float s = hsv[1];
    if( s < 0. )
        s = 0.;
    if( s > 1. )
        s = 1.;
    
    float v = hsv[2];
    if( v < 0. )
        v = 0.;
    if( v > 1. )
        v = 1.;
    
    // if sat==0, then is a gray:
    
    if( s == 0.0 )
    {
        rgb[0] = rgb[1] = rgb[2] = v;
        return;
    }
    
    // get an rgb from the hue itself:
    
    float i = floor( h );
    float f = h - i;
    float p = v * ( 1.f - s );
    float q = v * ( 1.f - s*f );
    float t = v * ( 1.f - ( s * (1.f-f) ) );
    
    float r, g, b;            // red, green, blue
    switch( (int) i )
    {
        case 0:
            r = v;    g = t;    b = p;
            break;
            
        case 1:
            r = q;    g = v;    b = p;
            break;
            
        case 2:
            r = p;    g = v;    b = t;
            break;
            
        case 3:
            r = p;    g = q;    b = v;
            break;
            
        case 4:
            r = t;    g = p;    b = v;
            break;
            
        case 5:
            r = v;    g = p;    b = q;
            break;
    }
    
    
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}

//open file
int     ReadInt(FILE *);
short   ReadShort(FILE *);

struct bmfh {
    short bfType;
    int bfSize;
    short bfReserved1;
    short bfReserved2;
    int bfOffBits;
} FileHeader;

struct bmih {
    int biSize;
    int biWidth;
    int biHeight;
    short biPlanes;
    short biBitCount;
    int biCompression;
    int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    int biClrUsed;
    int biClrImportant;
} InfoHeader;

const int birgb = { 0 };

int
main( int argc, char *argv[ ] )
{
    // turn on the glut package:
    // (do this before checking argc and argv since it might
    // pull some command line arguments out)
    srand(clock());
    rand();
    glutInit( &argc, argv );
    
    
    // setup all the graphics stuff:
    
        InitGraphics( );
    
    
        // create the display structures that will not change:
    
        InitLists( );
    
    
        // init all the global variables used by Display( ):
        // this will also post a redisplay
    
        Reset( );
    
    
        // setup all the user interface stuff:
    
        InitMenus( );
    
    
        // draw the scene once and wait for some interaction:
        // (this will never return)
    
        glutSetWindow( MainWindow );
        glutMainLoop( );
    
    // this is here to make the compiler happy:
    
    return 0;
}
