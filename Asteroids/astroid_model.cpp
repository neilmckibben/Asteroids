#include <iostream>
#include <string>
#include <algorithm>
#include <stdio.h>
using namespace std;
#include "olcConsoleGameEngine.h"

// TODO: does this need to inherit game engine?
class astroid_model : public olcConsoleGameEngine {

private:

	struct spaceObject {
		int size;
		float x;
		float y;
		float dx;
		float dy;
		float angle;

	};

	vector<spaceObject> asteroids;
	vector<spaceObject> bullets;
	spaceObject player;
	// player state will dictate temporary invincibility after collision
	enum PlayerState { Full, Damaged, Dead };
	PlayerState currState;
	// invCounter is the invincibility counter representing how many calls to onUserUpdate the player
	// will be invincible for
	// TODO: possibly change invCounter number
	int invCounter;
	bool dead = false;
	int nScore = 0;

	std::vector<pair<float, float>> ship_model;
	std::vector<pair<float, float>> vec_astroid_model;


public:

	spaceObject getPlayer() {
		return player;
	}

	int getScore() {
		return nScore;
	}

	std::vector<pair<float, float>> getShipModel() {
		return ship_model;
	}

	std::vector<pair<float, float>> getAsteroidModel() {
		return vec_astroid_model;
	}

	std::vector<spaceObject> getBullets() {
		return bullets;
	}

	std::vector<spaceObject> getAsteroids() {
		return asteroids;
	}

	bool OnUserCreate() {
		ship_model = {
			{0.0f, -4.0f},
			{-2.5f, 2.5f},
			{2.5f, 2.5f}
		};

		int points = 30;
		for (int i = 0; i < points; i++) {
			float standard_dev = (float)rand() / (float)RAND_MAX * 0.3f + 0.9f;
			vec_astroid_model.push_back(make_pair(standard_dev*sinf(((float)i / (float)points) * 6.28318f),
				standard_dev * cosf(((float)i / (float)points) * 6.28318f)));

		}

		resetGame();

		return true;
	}

	void resetGame() {
		player.x = ScreenWidth() / 2.0f;
		player.y = ScreenWidth() / 2.0f;
		player.dx = 0.0f;
		player.dy = 0.0f;
		player.angle = 0.0f;

		bullets.clear();
		asteroids.clear();

		asteroids.push_back({ (int)16, 20.0f, 20.0f, 8.0f, -6.0f, 0.0f });
		asteroids.push_back({ (int)16, 100.0f, 20.0f, -5.0f, 3.0f, 0.0f });

		invCounter = 0;
		currState = Full;
		dead = false;
		nScore = 0;
	}

	void wrapCords(float x, float y, float &out_x, float &out_y) {
		out_x = x;
		out_y = y;
		if (x < 0.0)
			out_x = x + (float)ScreenWidth();
		if (x > (float)ScreenWidth())
			out_x = x - (float)ScreenWidth();
		if (y < 0.0)
			out_y = y + (float)ScreenHeight();
		if (y > (float)ScreenHeight())
			out_y = y - (float)ScreenHeight();
	}

	/* -- Collision Detection -- Connors part */

	// function for checking whether an asteroid and another space object collided
	bool IsPointInsideSpaceObj(float posX1, float posY1, float radius, float posX2, float posY2)
	{
		return sqrt((posX2 - posX1)*(posX2 - posX1) + (posY2 - posY1)*(posY2 - posY1)) < radius;
	}

	// Check if player model intersects with any objects in asteroid vector
	bool detect_player_collision()
	{

		for (auto &asteroid : asteroids)
		{
			return IsPointInsideSpaceObj(asteroid.x, asteroid.y, asteroid.size, player.x, player.y);
		}
	}

	// handles collision based on player states
	void handle_collision()
	{
		if (currState = Full)
		{
			currState = Damaged;
			invCounter = 100;
		}
		else if (currState = Damaged)
		{
			currState = Dead;
			dead = true;
		}
	}

	// detects and handles bullet collisions with asteroids
	void detect_bullet_collision(spaceObject &asteroid, spaceObject &bullet, std::vector<spaceObject> &asteroidsHit)
	{
		// designate bullet to be removed by removal algorithm
		bullet.x = -100;

		// split asteroids if size > 4
		if (asteroid.size > 4)
		{
			float angle1 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
			float angle2 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
			// push two new asteroids, in random angles with size bitshifted 'down' by 1
			asteroidsHit.push_back({ (int)asteroid.size >> 1, asteroid.x, asteroid.y, 10.0f * sinf(angle1), 10.0f * cosf(angle1), 0.0f });
			asteroidsHit.push_back({ (int)asteroid.size >> 1, asteroid.x, asteroid.y, 10.0f * sinf(angle2), 10.0f * cosf(angle1), 0.0f });
		}

		// designate remove old asteroid
		asteroid.x = -100;
		nScore += 100;
	}

	// handler for level cleared
	void handle_level_clear()
	{
		nScore += 1000;
		asteroids.clear();
		bullets.clear();

		// add two more asteroids so that their coordinates don't intersect with the player
		// coordinate wrap will be applied on next user update
		asteroids.push_back({ (int)16, 30.0f * sinf(player.angle - 3.14159f / 2.0f) + player.x,
										  30.0f * cosf(player.angle - 3.14159f / 2.0f) + player.y,
										  10.0f * sinf(player.angle), 10.0f*cosf(player.angle), 0.0f });

		asteroids.push_back({ (int)16, 30.0f * sinf(player.angle + 3.14159f / 2.0f) + player.x,
										  30.0f * cosf(player.angle + 3.14159f / 2.0f) + player.y,
										  10.0f * sinf(-player.angle), 10.0f*cosf(-player.angle), 0.0f });
	}

	// wills main part
	void update_movement(float fElapsedTime) {

		if (m_keys[VK_LEFT].bHeld) {// left key held
			player.angle -= 6.0 * fElapsedTime;
		}
		if (m_keys[VK_RIGHT].bHeld) {// right key held
			player.angle += 6.0 * fElapsedTime;
		}
		if (m_keys[VK_UP].bHeld) {// thrust
			player.dx += sin(player.angle) * 20.0f * fElapsedTime;
			player.dy += -cos(player.angle) * 20.0f * fElapsedTime;
		}

		player.x += player.dx * fElapsedTime;
		player.y += player.dy * fElapsedTime;

		wrapCords(player.x, player.y, player.x, player.y);

		if (m_keys[VK_SPACE].bReleased) {
			bullets.push_back({ 0, player.x, player.y, 50.0f * sinf(player.angle), -50.0f * cosf(player.angle), 100.0f });
		}

		for (auto &a : asteroids) {
			a.x += a.dx * fElapsedTime;
			a.y += a.dy * fElapsedTime;
			a.angle += 0.5f * fElapsedTime;
			wrapCords(a.x, a.y, a.x, a.y);
		}

		/* asteroids hit by player will be stored in a temporary vectory so as to not affect iteration
		on current asteroid vector. They will then be split and later placed in current asteroid vector
		or destroyed and removed by removal algorithm */
		std::vector<spaceObject> asteroidsHit;

		for (auto &b : bullets) {
			b.x += b.dx * fElapsedTime;
			b.y += b.dy * fElapsedTime;
			wrapCords(b.x, b.y, b.x, b.y);
			b.angle -= 1.0f * fElapsedTime;

			// detect bullet-asteroid collisions
			for (auto &asteroid : asteroids)
			{
				if (IsPointInsideSpaceObj(asteroid.x, asteroid.y, asteroid.size, b.x, b.y))
				{
					detect_bullet_collision(asteroid, b, asteroidsHit);
				}
			}
		}

		// add newly created asteroids to current asteroid vector
		for (auto a : asteroidsHit)
		{
			asteroids.push_back(a);
		}

		// remove old asteroid objects
		for (auto &a : asteroids)
		{
			if (a.x < 1 || a.y < 1 || a.x > ScreenWidth() || a.y > ScreenHeight())
			{
				asteroids.erase(a);
			}
		}

		// TODO: do you need an iteration condition in the for loop?
		std::vector<spaceObject>::iterator it;
		for (it = bullets.begin(); it != bullets.end();) {
			if (it->x < 1 || it->y < 1 || it->x > ScreenWidth() || it->y > ScreenHeight()) {
				it = bullets.erase(it);
			}
			else {
				it++;
			}
		}

	}


	bool OnUserUpdate(float fElapsedTime) {
		if (dead) {
			resetGame();
		}
		// bullet collisions with asteroids are done at the same time bullet movements are updated
		update_movement(fElapsedTime); // will

		// check for ship collisions with asteroids -- connor
		if (detect_player_collision() && invCounter == 0)
		{
			handle_collision();
		}

		// decrememnt invincibility counter on every userUpdate
		if (invCounter != 0)
		{
			invCounter--;
		}

		// update for level cleared
		if (asteroids.empty())
		{
			handle_level_clear();
		}

		/* I set a state for Dead in the enum which is different than the 'dead' bool so the view can handle any
		death animations/view updates before calling resetGame() */
		// update view, neil

	}

};