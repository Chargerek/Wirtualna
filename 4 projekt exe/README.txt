========================================
PROGRAM OPENGL2 - INSTRUKCJA UŻYTKOWANIA
========================================

Plik wykonywalny: OpenGL2.exe

WYMAGANE PLIKI (wszystkie muszą być w tym samym katalogu):
- OpenGL2.exe (główny plik programu)
- glfw3.dll (biblioteka GLFW)
- folder "shaders" z plikami shaderów:
  * diffuse.vert, diffuse.frag
  * specular.vert, specular.frag
  * blinn_phong.vert, blinn_phong.frag
  * texture.vert, texture.frag
  * flag.vert, flag.frag

STEROWANIE:
- W, S, A, D - poruszanie kamerą (przód, tył, lewo, prawo)
- SPACE, C - poruszanie kamerą w górę/dół (w trybie kamery)
- Mysz - obracanie kamery
- L - przełączanie między trybem kamery a trybem światła
- ESC - zamknięcie programu

W TRYBIE ŚWIATŁA:
- W, S - poruszanie światłem w osi Z (-Z, +Z)
- A, D - poruszanie światłem w osi X (-X, +X)
- SPACE, C - poruszanie światłem w osi Y (+Y, -Y)

OPIS PROGRAMU:
Program wyświetla 5 obiektów 3D z różnymi modelami oświetlenia:
1. Model światła rozproszonego (diffuse) - niebieski sześcian
2. Model światła odbitego (specular) - czerwony sześcian
3. Model Blinna-Phonga - zielony sześcian
4. Teksturowanie bez oświetlenia - żółty sześcian
5. Efekt falowania flagi - fioletowa płaszczyzna

Światło punktowe jest wizualizowane jako żółta kostka.

========================================
WERSJA: Release x64
DATA KOMPILACJI: 12.12.2025
========================================

