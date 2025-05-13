#pragma once

class Manager {
public:
	Manager();
	~Manager();

	bool Update(double dt);
	void Draw();

	void CleanUp();
private:

};
