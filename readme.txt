Pour compiler, simplement exécuter le fichier bat.
Il est aussi recommandé d'ouvrir le projet dans visual studio comme projet CMAKE.

--- Format de soumission (correction automatique) ---
Soumettre un fichier .zip contenant votre projet CMake.
- Le zip doit contenir directement le CMakeLists.txt et les sous-dossiers comme /src à la racine.
- Évitez d'inclure les dossiers/fichiers générés: build/, Build/, CMakeFiles/, _deps/, .exe, .obj, .pdb.

Dans visual studio, choisir infographie1.vcxproj comme Startup Item.

Il est possible de compiler en ligne de commande comme suit:
```cmake --build ".\build" --config Release```

Une fois le projet compilé, il est possible de rouler les tests avec la commande suivante:
```ctest --test-dir build -C Release```

---
Linux (ex: Ubuntu)

Dépendances typiques:
 - cmake
 - compilateur C++ (g++, clang)
 - OpenGL + X11 (ex: libgl1-mesa-dev, xorg-dev)

Configuration/compilation:
```cmake -S . -B build -DCMAKE_BUILD_TYPE=Release```
```cmake --build build```

Tests:
```ctest --test-dir build```