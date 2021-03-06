#pragma once
#include "control/th_player.h"

/* Visualization Constants */
static const float VEC_FIELD_MIN_RESOLUTION = 8.f;
static const float MAX_FRAMES_TILL_COLLISION = 10.f;	// used for coloring vector field

/* Algorithmic Constants */
static const float SQRT_2 = sqrt(2.f);
static const float MIN_SAFETY_TICK = 10.0f;


/**
 * \brief Implementation of velocity obstacle based algorithm
 *
 * Overview:
 * Implementation of constrained linear velocity obstacle algorithm 
 * using a linear approximation of bullet trajectories.
 * 
 * Uses an AABB predictor for bullet hitboxes, OBB SAT predictor for lasers, 
 * and circle predictor for hitcircles. Ideally we want to use a predictor that 
 * corresponds to the collision algorithm used by the games, but some of them 
 * cannot be projected easily.
 * 
 * This algorithm performs quite well for just dodging obstacles, since most 
 * bullets are linear in nature. Problems arise with obstacles that are non-linear, 
 * or bullet patterns that require memorization to pass. This algorithm usually bombs 
 * through these parts.
 * 
 * Note that targeting is also difficult, since there is no simple way to weight 
 * targets and obstacles in a reasonable way. Currently the algorithm only targets 
 * powerups will be no obstacles blocking the way which should work in theory, 
 * but causes the bot to collide with obstacles more often.
 * 
 * This algorithm implements near perfect deathbombing by detecting if it will collide 
 * with an obstacle very soon, and has no recourse for avoidance. However, this is 
 * dependent on the linear approximation and the accuracy of the collision predictors. 
 *
 * High-Level Function:
 * First we must calibrate the algorithm by determining the player velocity, by
 * frame division.
 *
 * Each velocity state then projected for collisions with obstacles. The state that
 * results in a collision being the furthest away (greedy) is the desired action.
 */
class th_vo_algo : public th_algorithm
{
	/* Adaptibility Parameters */
	// Should we use hitcircles instead of hitboxes
	bool hitCircle = false;

	/* Calibration Parameters */
	bool isCalibrated = false;
	// Number of frames spent calibrating so far
	int calibFrames = 0;
	// Starting x position when beginning calibration
	float calibStartX = -1;
	float playerVel = 0;
	float playerFocVel = 0;

	void calibInit();

	// Get player's movement vector when moving in this direction
	vec2 getPlayerMovement(int dir);

	/**
	* \brief Do one tick of calibration
	* \return Whether calibration is complete or not
	*/
	bool calibTick();

	/* Visualization Parameters*/

	bool renderVectorField = false;

	/**
	 * \brief Find the minimum collision tick of a static AABB
	 * \param bullets The bullets to check collision against
	 * \param p Top-left corner of AABB to check
	 * \param s Size of AABB to check
	 * \param collided All bullets which collide with the AABB are added to this vector
	 * \return The minimum collison tick
	 */
	float minStaticCollideTick(
		const std::vector<const game_object*> &bullets,
		const aabb &area,
		std::vector<const game_object*> &collided) const;
	/**
	 * \brief Draw collision potentials at a specified resolution
	 * \param bullets The bullets to check collision against
	 * \param p Position of AABB containing visualization boundary
	 * \param s Size of AABB containing visualization boundary
	 * \param minRes Minimum allowable resolution for visualization
	 */
	void vizPotentialQuadtree(
		const std::vector<const game_object*> &bullets,
		const aabb &area,
		float minRes) const;

	std::vector<const game_object*> constructDangerObjectUnion();
	/* IMGUI Integration */
	static const int RISK_HISTORY_SIZE = 90;
	float riskHistory[RISK_HISTORY_SIZE] = {0};

public:
	th_vo_algo(th_player *player) : th_algorithm(player) {}
	th_vo_algo(th_player *player, bool hit_circle) : th_algorithm(player), hitCircle(hit_circle) {}

	~th_vo_algo() = default;

	void onBegin() override;
	void onTick() override;
	void visualize(IDirect3DDevice9 *d3dDev) override;
};
