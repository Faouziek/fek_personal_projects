# Corpus de démonstration

**Think Python — Chapter-based glossary**, disponible dans le dépôt d’Allen B. Downey :
https://github.com/AllenDowney/ThinkPython2/blob/e2d8acc138061ac3f17f2b9c12f42b0c7d3855ef/Think%20Python%20Glossary%20by%20Chapter.pdf

Un PDF de 15 pages, en anglais, couvrant programmation, fonctions, récursion,
structures de données, objets et complexité algorithmique. Ce corpus volontairement
petit permet une démonstration CPU et des références vérifiables manuellement.
Le modèle d’embeddings multilingue permet de poser des questions en français.

`manifest.json` fixe la révision amont, l’URL, le SHA-256 et le nombre de pages.
`rag download` vérifie le téléchargement ; `rag index` vérifie à nouveau les PDF.
Les pages citées sont les **pages physiques du PDF, à partir de 1**.
Le document téléchargé reste dans `data/pdfs/`, hors Git.

## Attribution et droits

Livre source : Allen B. Downey, *Think Python*, Green Tea Press.
Le dépôt amont contient une licence MIT pour le logiciel ; le fichier `book/book.tex`
indique **Creative Commons Attribution–NonCommercial 3.0 Unported** pour le livre :
https://creativecommons.org/licenses/by-nc/3.0/

Le glossaire se présente comme une compilation de ce livre. Nous conservons cette
attribution et retenons les conditions non commerciales du livre pour ce corpus
dérivé, sans lui attribuer la licence MIT du code. Le PDF est téléchargé depuis
l’amont, sans modification ni redistribution dans ce projet. Pour un usage commercial,
choisir un autre corpus dont les droits le permettent ou obtenir l’autorisation.

## Ajouter ses PDF

Ajouter dans `manifest.json` une entrée par document avec un `id` unique, un nom de
fichier simple, une URL HTTPS, le SHA-256 attendu et le nombre de pages. Déposer les
PDF privés directement dans `data/pdfs/` évite de les publier : ils doivent néanmoins
correspondre au manifeste. Ne pas versionner de documents confidentiels.
Relancer l’indexation et créer des annotations propres au nouveau corpus.
Les PDF scannés nécessitent un OCR préalable ; cette démo n’intègre pas d’OCR.
