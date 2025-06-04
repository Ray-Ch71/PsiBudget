# PsiBudget – Gestion budgétaire étudiante

**PsiBudget** est une application de bureau développée en langage C, reposant sur l’API Win32, qui permet aux étudiants de suivre leurs revenus et leurs dépenses de manière simple, rapide et efficace. Ce projet a été réalisé dans le cadre de notre première année au sein de la formation CPB à l’ENSISA (Université de Haute-Alsace).

## Présentation générale

Le logiciel propose une interface graphique minimaliste qui permet à l’utilisateur d’ajouter une source de revenu ou de dépense, de visualiser la liste complète des transactions, de suivre son solde en temps réel, ainsi que d’accéder à une représentation graphique des dépenses sous forme de diagramme circulaire.

Les données sont enregistrées localement dans un fichier texte, permettant une persistance des informations d’une session à l’autre. L’objectif principal est de fournir un outil léger et pédagogique, pensé pour les besoins concrets d’un étudiant nouvellement autonome.

## Fonctionnalités développées

Le cœur de l’application repose sur une gestion dynamique des transactions avec une interface Win32 composée de fenêtres distinctes pour chaque fonctionnalité. Le programme gère l’ajout de données, leur affichage formaté, et la visualisation graphique de la répartition des dépenses par catégorie. Le calcul du solde s’effectue automatiquement à partir des données du fichier.

## Structure du projet

Le projet est organisé autour d’un fichier principal `main.c` contenant l’ensemble des fonctions nécessaires : création de fenêtres, gestion des événements utilisateur, lecture/écriture dans le fichier `transactions.txt`, rendu graphique du camembert, etc.

Des dossiers complémentaires (`docs/`, `screenshots/`) peuvent contenir les supports de présentation, les rapports ou les captures d’écran de l’interface.

## Lancement de l’application

Pour exécuter PsiBudget, il suffit d’ouvrir le fichier `main.c` dans un environnement de développement compatible C sous Windows (comme Code::Blocks ou Visual Studio), puis de compiler et exécuter le programme. Aucune installation supplémentaire n’est requise.

## Perspectives d’évolution

Plusieurs pistes d’amélioration sont envisagées : exportation des données au format PDF, support multilingue, amélioration de l’ergonomie graphique, ajout de filtres et tris dans l’historique, ainsi qu’une future intégration de l’intelligence artificielle pour l’analyse automatique des dépenses et la prévision budgétaire.

## Auteurs

Rayan Chammakhi  
Mohamed Dhia Eddine Selmi  
CPB1 – ENSISA – Année 2024/2025

Encadrant :Corrine Jung

## Licence

Ce projet est distribué sous licence MIT. Vous pouvez l’utiliser, le modifier et le diffuser librement, sous réserve de mentionner les auteurs d’origine.
