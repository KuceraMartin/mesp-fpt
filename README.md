# Minimum Eccentricity Shortest Path
This is an implementation of the MESP FPT algorithm parametrized by the distance to disjoint paths and minimum eccentricity, combined.

This project uses Boost library version 1.72. It can be built using cmake:
```bash
export BOOST_ROOT=<path-to-boost>
cmake -S . -B <path-to-build>
cmake --build <path-to-build> --target all
```

There are three targets: `mesp`, `paths`, and `test`.
The main program which solves the MESP problem is `mesp`.
It requires the modulator to disjoint paths as an input file, which can be calculated by `paths`.
The `test` target is used for testing purposes.  
