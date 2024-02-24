# Space Invaders 2: Electric Boogaloo
 
Excusing the misleading name, this game is actually a clone of the old arcade shooter, Asteroids _(albeit with improvements to the physics, graphics, & gameplay features)_\
Assignment 1 of my "Programming Physics" module that I took during my time in Nanyang Polytechnic (NYP). Gameplay can be found [here](https://youtu.be/mf0ykPw7NUE?si=UiMnmGYqPR48u1Vc)

### Features
* Physics adheres more closely to newtonian mechanics, compared to the classic asteroids
  * Movement to the player ship is controlled via a Force Vector being applied to the player ship's acceleration
  * Similarly, turning is controlled via torque being applied to the angular velocity of the ship
  * Friction _(yes, I know there is not much friction in space, but it improves the gameplay)_, to slow down the ship over time
* Wave-Survival system, with each wave getting progressively harder 
* Enemy AI that will lock on to the player
* Friend AI ship that will help you in killing enemies
* Random events
  * A Saturn-like planet will spawn ocassionally, with a physically accurate _gravitional pull_. Death upon impact

### Controls
* Movement
  * W - Move foward
  * A - Rotate counter-clockwise
  * D - Rotate clockwise
  * S - Move backward

* Weapons
  * Q/E - Cycle through weapons
  * LMB/M1 - Fire
  * Spacebar - Bullet Repulsar

* Miscellaneous
  * B - Spawn another Buddy Ship
