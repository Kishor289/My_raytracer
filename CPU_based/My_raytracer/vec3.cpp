#include "vec3.h"

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






