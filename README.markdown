Readme: Shiny Robots
====================

List of Files
-------------
(and what they do)
In mostly alphabetical order:

- app.h:
	- defines the application class.
	- this class looks after windowing and input, and also holds a pointer to the current scene.
	- (effectively a state machine.)
- app.cpp:
	- contains implementation of the application class.
	- constructor opens window and sets input callbacks.
	- update function checks controls and updates the current scene.
	- render function resizes the viewport, clears the screen and renders the current scene.
- bsm.h
	- header file for ROBO_DONUT's Binary Static Mesh library
	- https://github.com/ml32/Binary-Static-Mesh
- game.h
	- defines the game class, which manages the state of the currently running game.
	- inherits from the abstract scene class.
- game.cpp
	- contains the game implementation, and a few lisp functions.
	- constructor sets up the camera and loads the default level
	- level is loaded from json file inside level directory containing the level assets.
- main.cpp
	- contains main function/entry point.
	- instantiates a new app object, and calls its mainloop function.
- model.h
	- provides a wrapper class around an OpenGL IBO and VBO.
	- loaded from a filename, provides a render function that binds the buffers and pushes the geometry.
- model.cpp
	- contains implementation of model class
- phys_obj.h
	- wrapper around a Bullet lib rigid body.
	- provides a render method that draws the attached model
- phys_obj.cpp
	- physObj implementation
- scene.h
	- abstract base class that all scenes inherit from
	- has pure virtual render and update methods
	- also defines a sceneInfo struct that contains key state etc. (passed from application class)
- util.h
	- declares utility functions used by other parts of the application
	- includes functions for creating bullet meshes/hulls from BSM files, and for extracting data from JSON objects (such as vectors/quaternions, or entire assemblies)
- util.cpp
	- implements the utility functions
- workshop.h
	- declares the workshop scene, which allows users to place parts and build robots.
	- inherits from scene: update/render functions are called from the application class.
- workshop.cpp
	- implements the workshop scene.
- world.h
	- declares a wrapper around a Bullet world
	- keeps a hold of all of the necessary components, cleans them up when it's destroyed.
- world.cpp
	- implements the world class and tag dictionary class