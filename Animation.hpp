#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <iostream>
#include "ft_vec.hpp"

typedef ft::vector<float> vec;

class Animation
{
private:
	vec translation;
	vec rotation;
	vec scale;
	vec color;

public:
	Animation();
	Animation(vec translation, vec rotation, vec scale, vec color);

	const vec getTranslation() const;
	const vec getRotation() const;
	const vec getScale() const;
	const vec getColor() const;
	bool isValid() const;

	friend std::ostream &operator<<(std::ostream &os, const Animation &animation);
	friend std::istream &operator>>(std::istream &is, Animation &animation);
	friend Animation linear_interpolation(const Animation &a, const Animation &b, float t);
};

#endif
