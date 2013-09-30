Comparing of Grid and nonoptimized approach
===========================================

Let's note N as number of particles, then Grid ([spatial partition](http://gameprogrammingpatterns.com/spatial-partition.html)) is near to `O(N)`, while straightforward approach gives us pure `O(N^2)`.

Here is a list of experimental results. Result is output of a command like `time ./simulation...` for different approaches.

## Grid has big constant and can't show supremacy at small number of particles:

**relaxation = 500, number_of_particles: 100**
	
- naive: `0m1.495s`
- grid: `0m14.233s`

## Straightforward approach and Grid are about to be on par

**relaxation = 50, number_of_particles: 1000**

- naive: `0m11.112s`
- grid: `0m14.422s`

## Here Grid is substantially faster

**relaxation = 5, number_of_particles: 10000**
	
- naive: `1m46.656s`
- grid: `0m14.735s`
