
# Projet : Interface de gestion des threads (type `pthread.h`)

Ce projet implémente une interface de gestion des threads similaire à celle de la bibliothèque `pthread.h`. Il comprend également une série de tests, ainsi que des fonctionnalités avancées comme :

- La préemption
- La synchronisation via des mutex
- La gestion des signaux
- La détection des deadlocks

## 🔧 Compilation

### Compilation avec notre propre interface de threads
```bash
make
```

### Compilation avec l’interface `pthread`
```bash
make pthread
```

## ▶️ Exécution

Après compilation, le binaire principal peut être exécuté avec :
```bash
./install/bin/executable
```

## 📊 Génération de graphiques

Pour générer des graphes basés sur les performances ou comportements des threads :
```bash
python3 graph.py executables --args
```

> Remplacez `--args` par les arguments spécifiques à passer aux exécutables.

## 📁 Structure du projet (optionnelle)
- `src/controlleur/` : Fichiers source principaux
- `test/` : Fichiers de tests
- `build/` : Fichiers compilés
- `install/bin/` : Emplacement des exécutables
- `graph.py` : Script de génération de graphes



