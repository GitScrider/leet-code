<sub>[🧠 Root](../../README.md) · [⚙️ Algorithms](../README.md) · **➗ Math & Number Theory**</sub>

# ➗ Math & Number Theory

![algorithms](https://img.shields.io/badge/algorithms-9-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Core number-theory and combinatorics building blocks: GCD, primes and sieves, modular arithmetic, factorization, primality testing, and fast exponentiation.

| File | Complexity | Description |
|---|---|---|
| [gcd_lcm_extended.cpp](gcd_lcm_extended.cpp) | O(log n) | GCD/LCM plus extended Euclidean algorithm |
| [sieve_of_eratosthenes.cpp](sieve_of_eratosthenes.cpp) | O(n log log n) | Prime sieve up to n |
| [fast_exponentiation.cpp](fast_exponentiation.cpp) | O(log n) | Binary/modular exponentiation |
| [modular_inverse.cpp](modular_inverse.cpp) | O(log n) | Modular inverse via Fermat + extended Euclid |
| [prime_factorization.cpp](prime_factorization.cpp) | O(√n) | Trial-division prime factorization |
| [miller_rabin.cpp](miller_rabin.cpp) | O(k log³ n) | Deterministic primality test (fixed bases {2..37}, exact for 64-bit n) |
| [combinatorics.cpp](combinatorics.cpp) | O(n) | nCr, Pascal's triangle, nCr mod p |
| [euler_totient.cpp](euler_totient.cpp) | O(√n) / sieve | Euler's totient φ(n) single value and sieve |
| [matrix_exponentiation.cpp](matrix_exponentiation.cpp) | O(log n) | Matrix power for fast Fibonacci / recurrences |

## Build
```bash
g++ -std=c++17 -Wall -Wextra sieve_of_eratosthenes.cpp -o demo && ./demo
```
