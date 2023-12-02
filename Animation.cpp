#include "Animation.hpp"

Animation::Animation() : translation(vec(3)), rotation(vec(3)), scale(vec(3)), color(vec(3))
{
}

Animation::Animation(vec translation, vec rotation, vec scale, vec color)
	: translation(translation), rotation(rotation), scale(scale), color(color)
{
}

const vec Animation::getTranslation()
{
	return translation;
}

const vec Animation::getRotation()
{
	return rotation;
}

const vec Animation::getScale()
{
	return scale;
}

const vec Animation::getColor()
{
	return color;
}

std::ostream &operator<<(std::ostream &os, const Animation &animation)
{
	os << animation.translation << ";"
	   << animation.rotation << ";"
	   << animation.scale << ";"
	   << animation.color;
	return os;
}

std::istream &operator>>(std::istream &is, Animation &animation)
{
	char separator;
	is >> animation.translation;
	is >> separator;
	is >> animation.rotation;
	is >> separator;
	is >> animation.scale;
	is >> separator;
	is >> animation.color;
	return is;
}

Animation linear_interpolation(const Animation &a, const Animation &b, float t)
{
	return Animation(
		linear_interpolation(a.translation, b.translation, t),
		linear_interpolation(a.rotation, b.rotation, t),
		linear_interpolation(a.scale, b.scale, t),
		linear_interpolation(a.color, b.color, t));
}
