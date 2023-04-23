#pragma once

class Timestep {
public:
	Timestep(float time = 0.0f) : _time(time), _prev_time(0.0f) {
	}

	float GetSeconds() const { return _time; }
	float GetMilliseconds() const { return _time * 1000.0f; }

	void UpdateTime(float time)
	{
		_time = time - _prev_time;
		_prev_time = time;
	}

	operator float() const {
		return _time;
	}
private:
	float _time;
	float _prev_time;
};