#include <iostream>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <fstream>
#include <ctime>
using namespace std;
#include "olcConsoleGameEngine.h"

#define INVINCIBLE_TIME 5
#define ASTEROID_SPEED 10.0f

class astroid_model : public olcConsoleGameEngine
{

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
	enum PlayerState { Full, Damaged, Invincible };
	PlayerState currState;
	bool dead = false;
	int Score = 0;
	int level = 1;
	time_t inv_time;

	vector<pair<float, float>> ship_model;
	vector<pair<float, float>> vec_astroid_model;

	virtual bool OnUserCreate()
	{
		ship_model =
		{
			{ 0.0f, -5.0f},
			{-2.5f, +2.5f},
			{+2.5f, +2.5f}
		};

		int points = 20;
		for (int i = 0; i < points; i++)
		{
			float noise = (float)rand() / (float)RAND_MAX * 0.2f + 0.8f;
			vec_astroid_model.push_back(make_pair(noise * sinf(((float)i / (float)points) * 6.28318f),
				noise * cosf(((float)i / (float)points) * 6.28318f)));
		}

		resetGame();
		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime)
	{

		if (dead) {
			game_over_screen();
			if (m_keys[0x0D].bPressed) resetGame();
			else return true;
		}

		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, 0);

		time_t now;
		time(&now);
		if (currState == Invincible && difftime(now, inv_time) > INVINCIBLE_TIME) {
			currState = Damaged;
		}

		update_movement(fElapsedTime);

		if (currState == Full) {
			DrawWireFrameModel(ship_model, player.x, player.y, player.angle, 1.0f, FG_GREEN);
		}
		else if (currState == Damaged) {
			DrawWireFrameModel(ship_model, player.x, player.y, player.angle, 1.0f, FG_YELLOW);
		}
		else {
			DrawWireFrameModel(ship_model, player.x, player.y, player.angle, 1.0f, FG_WHITE);
		}

		if (detect_player_collision()) {
			handle_collision();
		}

		if (asteroids.empty()) {
			handle_level_clear();
		}

		DrawString(2, 2, L"SCORE: " + to_wstring(Score));
		return true;
	}


protected:

	void game_over_screen() {
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, 0);
		DrawString((ScreenWidth() / 2) - 4, ScreenHeight() / 2, L"SCORE: " + to_wstring(Score));
		DrawString((ScreenWidth() / 2) - 10, (ScreenHeight() / 2) + 1, L"Press enter to retry");
	}

	void resetGame() {
		player.x = ScreenWidth() / 2.0f;
		player.y = ScreenHeight() / 2.0f;
		player.dx = 0.0f;
		player.dy = 0.0f;
		player.angle = 0.0f;

		bullets.clear();
		asteroids.clear();
		level = 1;

		// Put in two asteroids
		asteroids.push_back({ (int)16, 20.0f, 20.0f, 8.0f, -6.0f, 0.0f });
		asteroids.push_back({ (int)16, 100.0f, 20.0f, -5.0f, 3.0f, 0.0f });

		// Reset game
		currState = Full;
		dead = false;
		Score = 0;
	}

	void wrapCords(float x, float y, float& out_x, float& out_y)
	{
		out_x = x;
		out_y = y;
		if (x < 0.0f)	out_x = x + (float)ScreenWidth();
		if (x >= (float)ScreenWidth())	out_x = x - (float)ScreenWidth();
		if (y < 0.0f)	out_y = y + (float)ScreenHeight();
		if (y >= (float)ScreenHeight()) out_y = y - (float)ScreenHeight();
	}

	bool IsPointInsideSpaceObj(float posX1, float posY1, float radius, float posX2, float posY2)
	{
		return sqrt((posX2 - posX1) * (posX2 - posX1) + (posY2 - posY1) * (posY2 - posY1)) < radius;
	}

	bool detect_player_collision()
	{
		for (auto& asteroid : asteroids)
		{
			if (IsPointInsideSpaceObj(asteroid.x, asteroid.y, asteroid.size, player.x, player.y)) {
				return true;
			}
		}
		return false;
	}

	void handle_collision()
	{
		if (currState == Full)
		{
			currState = Invincible;
			time(&inv_time);
		}
		else if (currState == Damaged)
		{
			dead = true;
		}
	}

	void detect_bullet_collision(spaceObject & asteroid, spaceObject & bullet, vector<spaceObject> & asteroidsHit)
	{
		if (asteroid.size > 4)
		{
			float angle1 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
			float angle2 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
			asteroidsHit.push_back({ (int)asteroid.size / 2, asteroid.x, asteroid.y, ASTEROID_SPEED * sinf(angle1), ASTEROID_SPEED * cosf(angle1), 0.0f });
			asteroidsHit.push_back({ (int)asteroid.size / 2, asteroid.x, asteroid.y, ASTEROID_SPEED * sinf(angle2), ASTEROID_SPEED * cosf(angle2), 0.0f });
		}

		bullet.x = -100;
		asteroid.x = -100;
		Score += 100;
	}

	void handle_level_clear()
	{
		Score += 1000;
		asteroids.clear();
		bullets.clear();
		level++;

		int inv = -1;

		for (int i = 0; i < level; i++) {
			asteroids.push_back({ (int)16, 30.0f * sinf(player.angle - 3.14159f / 2.0f) + player.x,
				30.0f * cosf(player.angle - 3.14159f / 2.0f) + player.y,
				-inv * ASTEROID_SPEED * sinf(player.angle), inv * ASTEROID_SPEED * cosf(player.angle), 0.0f });

			asteroids.push_back({ (int)16, 30.0f * sinf(player.angle + 3.14159f / 2.0f) + player.x,
				30.0f * cosf(player.angle + 3.14159f / 2.0f) + player.y,
				-inv * ASTEROID_SPEED * sinf(player.angle), inv * ASTEROID_SPEED * cosf(player.angle), 0.0f });
			inv *= -1;
		}

	}
	bool onUserCreate() {
		ship_model =
		{
			{ 0.0f, -5.0f},
			{-2.5f, +2.5f},
			{+2.5f, +2.5f}
		};

		int points = 20;
		for (int i = 0; i < points; i++)
		{
			float noise = (float)rand() / (float)RAND_MAX * 0.4f + 0.8f;
			vec_astroid_model.push_back(make_pair(noise * sinf(((float)i / (float)points) * 6.28318f),
				noise * cosf(((float)i / (float)points) * 6.28318f)));
		}

		resetGame();

		return true;
	}
	void update_movement(float fElapsedTime) {
		if (m_keys[0x25].bHeld) {
			player.angle -= 6.0 * fElapsedTime;
		}

		if (m_keys[0x27].bHeld) {
			player.angle += 6.0 * fElapsedTime;
		}

		if (m_keys[0x26].bHeld) {
			player.dx += sin(player.angle) * 20.0f * fElapsedTime;
			player.dy += -cos(player.angle) * 20.0f * fElapsedTime;

		}

		player.x += player.dx * fElapsedTime;
		player.y += player.dy * fElapsedTime;

		wrapCords(player.x, player.y, player.x, player.y);

		if (m_keys[0x20].bReleased) {
			bullets.push_back({ 0, player.x, player.y, 50.0f * sinf(player.angle), -50.0f * cosf(player.angle), 0.0f });
		}


		for (auto& a : asteroids)
		{
			a.x += a.dx * fElapsedTime;
			a.y += a.dy * fElapsedTime;
			a.angle += 0.5f * fElapsedTime;
			wrapCords(a.x, a.y, a.x, a.y);
			DrawWireFrameModel(vec_astroid_model, a.x, a.y, a.angle, (float)a.size, FG_YELLOW);
			//DrawCircle(a.x, a.y, a.size);
		}


		vector<spaceObject> asteroidsHit;


		for (auto& b : bullets)
		{
			b.x += b.dx * fElapsedTime;
			b.y += b.dy * fElapsedTime;
			wrapCords(b.x, b.y, b.x, b.y);

			// Check collision with asteroids
			for (auto& a : asteroids)
			{
				if (IsPointInsideSpaceObj(a.x, a.y, a.size, b.x, b.y))
				{
					detect_bullet_collision(a, b, asteroidsHit);
				}
			}
		}


		for (auto a : asteroidsHit)
		{
			asteroids.push_back(a);
		}

		// S/O stackoverflow
		asteroids.erase(std::remove_if(asteroids.begin(), asteroids.end(),
			[&](spaceObject i) { return i.x < 0; }), asteroids.end());

		for (auto& b : bullets) {
			Draw(b.x, b.y);
		}

		bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
			[&](spaceObject i) { return i.x < 1 || i.y < 1 || i.x >= ScreenWidth() - 1 || i.y >= ScreenHeight() - 1; }), bullets.end());

	}

	virtual void Draw(int x, int y, short c = 0x2588, short col = 0x000F) {
		float wx, wy;
		wrapCords(x, y, wx, wy);
		olcConsoleGameEngine::Draw(wx, wy, c, col);
	}

	void DrawWireFrameModel(const vector<pair<float, float>> & vecModelCoordinates, float x, float y, float r = 0.0f, float s = 1.0f, short col = FG_WHITE)
	{
		vector<pair<float, float>> vecTransformedCoordinates;
		int verts = vecModelCoordinates.size();
		vecTransformedCoordinates.resize(verts);

		// Rotate
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
			vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
		}

		// Scale
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
		}

		// Translate
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
		}

		// Draw Closed Polygon
		for (int i = 0; i < verts + 1; i++)
		{
			int j = (i + 1);
			DrawLine(vecTransformedCoordinates[i % verts].first, vecTransformedCoordinates[i % verts].second,
				vecTransformedCoordinates[j % verts].first, vecTransformedCoordinates[j % verts].second, PIXEL_SOLID, col);
		}
	}

};


int main()
{
	astroid_model game;
	game.ConstructConsole(160, 100, 8, 8);
	game.Start();
	return 0;
}
