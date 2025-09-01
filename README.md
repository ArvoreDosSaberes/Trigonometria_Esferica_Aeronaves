[![CI](https://github.com/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves/actions/workflows/ci.yml/badge.svg)](https://github.com/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves/actions/workflows/ci.yml)
![visitors](https://visitor-badge.laobi.icu/badge?page_id=ArvoreDosSaberes.Trigonometria_Esferica_Aeronaves)
[![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC_BY--SA_4.0-blue.svg)](https://creativecommons.org/licenses/by-sa/4.0/)
![Language: Portuguese](https://img.shields.io/badge/Language-Portuguese-brightgreen.svg)
[![Language-C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CMake](https://img.shields.io/badge/build-CMake-informational.svg)](https://cmake.org/)
[![Raylib](https://img.shields.io/badge/graphics-raylib-2ea44f.svg)](https://www.raylib.com/)
[![Issues](https://img.shields.io/github/issues/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves.svg)](https://github.com/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves/issues)
[![Stars](https://img.shields.io/github/stars/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves.svg)](https://github.com/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves/stargazers)
[![Forks](https://img.shields.io/github/forks/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves.svg)](https://github.com/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves/network/members)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://makeapullrequest.com)
[![Watchers](https://img.shields.io/github/watchers/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves)](https://github.com/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves/watchers)
[![Last Commit](https://img.shields.io/github/last-commit/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves)](https://github.com/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves/commits)
[![Contributors](https://img.shields.io/github/contributors/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves)](https://github.com/ArvoreDosSaberes/Trigonometria_Esferica_Aeronaves/graphs/contributors)

# Trigonometria Esférica aplicada a sensores em aeronaves (Raylib)

Visualizador interativo em C (Raylib) dos conceitos de Azimute/Elevação e do ângulo esférico J entre o eixo de rolagem da aeronave e a direção do alvo.

- Conversão Az/El -> vetor 3D unitário
- Cálculo do ângulo de grande círculo J entre dois vetores na esfera
- Esfera unitária, eixos N-E-Up e vetores R (rolagem) e T (alvo)
- Controles de câmera e de ajuste dos ângulos em tempo real

## Tutorial para iniciantes

Se você está começando agora, siga o passo a passo em: [TUTORIAL.md](TUTORIAL.md)

## Conceitos Implementados

- Azimute do vetor: de +X (Norte) para +Y (Leste), em radianos
- Elevação: do plano XY em direção a +Z (Up)
- Vetor do alvo T: `vT = (cos(ElT)cos(AzT), cos(ElT)sin(AzT), sin(ElT))`
- Vetor do eixo R: `vR = (cos(ElR)cos(AzR), cos(ElR)sin(AzR), sin(ElR))`
- Ângulo J via produto escalar: `J = arccos( clamp(vT·vR, -1, 1) )`
- Verificação por trigonometria esférica: `cos J = sin(ElT) sin(ElR) + cos(ElT) cos(ElR) cos(AzT - AzR)`

## Controles

- Alvo (T): A/D (Az −/+), W/S (El +/−)
- Eixo (R): J/L (Az −/+), I/K (El +/−)
- Reset: R
- Câmera: Botão direito do mouse e arraste para orbitar; scroll ajusta FOV

## Build

O projeto usa CMake e busca a dependência Raylib via FetchContent (clona do GitHub se não houver Raylib instalado no sistema).

Pré-requisitos no Linux:
- `build-essential` `cmake` `git`
- Bibliotecas X11: `libx11-dev libxrandr-dev libxi-dev libxinerama-dev libxcursor-dev`
- OpenGL: `mesa-common-dev libgl1-mesa-dev`

Passos:

```bash
cmake -S . -B build
cmake --build build -j
```

Executar:

```bash
./build/spherical_trig
```

## Estrutura

- `CMakeLists.txt`: configuração de build e Raylib
- `src/main.c`: renderização 3D, vetores T/R e matemática esférica (J)

## Licença

Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International. Veja o arquivo LICENSE no repositório principal.
