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

So...
-----
Get assemblies loading

- reuse level-loading code for statics, dynamics, constraints
 
Create workshop, assemble assemblies:

- load list of assemblies (dirent.h)
- allow them to be placed
- add make-on-place constraint semantics (?)

Wiring!

- define pins for each dynamic
- wire belongs to top-level assembly
- wire can access its own level and *lower* (child) assemblies
    - this avoids local collisions, but allows communication between sub-assemblies.
- wire has pointers to objects - no tag stuff. (tags are non-specific!)
- remove wires when we remove objects
