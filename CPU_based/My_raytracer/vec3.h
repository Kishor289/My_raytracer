#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

class vec3{
    public:
        //defining variable
        double e[3];

        //constructors
        vec3() : e{0, 0, 0} {}  //default constructor
        vec3(double x, double y, double z);

        //getters
        double x();
        double y();
        double z();

        //functions
        double length();
        vec3 normalized();
        double dot(vec3 v);

        vec3 perp_to(vec3 v){
            double d = *this * v.normalized();
            return *this - v.normalized()*d;

        }

        friend std::ostream& operator<<(std::ostream& out, const vec3& v); //print vector using outstream operator

        //operator overloads
        vec3 operator+(const vec3& v); //add vectors
        vec3 operator-(const vec3& v); // subtract vectors
        vec3 operator-(); //reverse a vector
        double operator*(const vec3& v); // dot
        vec3 operator*(double a); //multiply by scalar
        vec3 operator/(double a); //divide by a scalar
        vec3 operator^(const vec3& v); //cross product

};

vec3::vec3(double x, double y, double z ) : e{x, y, z}{}

double vec3::x(){
    return this->e[0];  
}

double vec3::y(){
    return this->e[1];
}

double vec3::z(){
    return this->e[2];
}

std::ostream& operator<<(std::ostream &out, const vec3 &v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

double vec3::length(){
    return sqrt(this->e[0]*this->e[0] + this->e[1]*this->e[1] + this->e[2]*this->e[2]);
}

vec3 vec3::normalized(){
    return vec3(this->e[0]/this->length(), this->e[1]/this->length(), this->e[2]/this->length());
}

vec3 vec3::operator+(const vec3& v)
{
    return vec3(v.e[0] + this->e[0], v.e[1] + this->e[1], v.e[2] + this->e[2]);
}

vec3 vec3::operator-(const vec3& v)
{
    return vec3(-v.e[0] + this->e[0], -v.e[1] + this->e[1], -v.e[2] + this->e[2]);
}

vec3 vec3::operator-()
{
    return vec3(-this->e[0], -this->e[1], -this->e[2]);
}

double vec3::operator*(const vec3& v)
{
    return v.e[0] *this->e[0] + v.e[1] * this->e[1] + v.e[2] * this->e[2];
}

vec3 vec3::operator*(double a)
{
    return vec3(a*this->e[0] ,  a * this->e[1] , a * this->e[2]);
}

vec3 vec3::operator/(double a){
    return vec3(this->e[0] /a ,  this->e[1] /a, this->e[2] /a);
}

vec3 vec3::operator^(const vec3& v){
    return vec3(this->e[1] * v.e[2] - this->e[2] * v.e[1],
                this->e[2] * v.e[0] - this->e[0] * v.e[2],
                this->e[0] * v.e[1] - this->e[1] * v.e[0]);
}

vec3 elm_multiply(vec3 u, vec3 v){
    return vec3(u.x()*v.x(), u.y()*v.y(), u.z()*v.z());
}

double vec3::dot(vec3 v){
    return v.e[0] *this->e[0] + v.e[1] * this->e[1] + v.e[2] * this->e[2];
}


#endif