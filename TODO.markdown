TODO
====
- Health
- explosions/particles
- guns
- ssao
- wiring
- assemblies
- building - workshop!
- proper programming
- scripts on per-assembly basis
- console
- proper levels
- scoring system
- online - download bots.
git a
So...
-----
Create workshop, assemble assemblies:

- add constraint tools
- tool class for separating tools and their states/resources from the workshop
- add make-on-place constraint semantics (?)

Wiring!

- define pins for each dynamic
- wire belongs to top-level assembly
- wire can access its own level and *lower* (child) assemblies
    - this avoids local collisions, but allows communication between sub-assemblies.
- wire has pointers to objects - no tag stuff. (tags are non-specific!)
- remove wires when we remove objects
