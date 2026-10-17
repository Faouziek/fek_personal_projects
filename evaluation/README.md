# Questions de référence

`questions.jsonl` contient **20 questions avec réponse documentaire** (15 françaises,
5 anglaises) et **3 questions hors corpus**. Chaque question positive indique une
réponse attendue, le document, une page PDF physique et une citation exacte, annotés
à partir du PDF avant l’évaluation. `test_corpus_annotations.py` vérifie ces citations
contre le véritable PDF lorsqu’il est téléchargé.

```sh
uv run --frozen rag evaluate -k 5
```

Rapport détaillé : `artifacts/evaluation.json`, avec classement et scores pour chaque
question. Les métriques portent sur les 20 questions positives :

- **Hit@K** : proportion de questions ayant au moins un extrait pertinent dans les K résultats.
- **MRR@K** : moyenne de l’inverse du rang du premier extrait pertinent, zéro en cas d’absence.
- **EvidenceRecall@K** : proportion moyenne des citations de référence retrouvées.

Un résultat est pertinent s’il possède le bon document, la bonne page **et contient
la citation attendue**, après normalisation Unicode, casse et espaces. Une autre
partie de la même page ne suffit pas. Une citation coupée entre deux chunks compte
comme un échec, même si la réponse serait reconstituable : cette convention stricte
rend le protocole reproductible mais peut sous-estimer la qualité réelle.

Les trois questions négatives apparaissent dans le rapport avec leurs voisins et
scores, mais n’entrent pas dans ces moyennes. Un index de plus proches voisins
retourne des résultats même hors sujet ; un score cosinus n’est pas une probabilité.
Aucun seuil universel d’abstention n’est supposé. Pour vérifier la génération :

```sh
uv run --frozen rag ask "Où une variable locale est-elle accessible ?"
uv run --frozen rag ask "Quel est le prix actuel du Bitcoin ?"
```

Vérifier manuellement l’exactitude de la réponse, l’appui de chaque affirmation par
ses citations et l’abstention sur les trois questions négatives. Les réponses de
référence sont fournies pour cette vérification ; elles ne sont jamais introduites
dans l’index ou le prompt de génération. L’évaluation automatique mesure la
**recherche**, pas la correction factuelle du LLM. Ce petit jeu de développement
n’est pas un benchmark indépendant pour optimiser des hyperparamètres puis annoncer
une performance de généralisation.

## Exécution de référence dans le cloud

Le [rapport de référence](baseline.json) consigne les versions et mesures réellement
exécutées sur Linux : **Hit@5 = 0,85 ; MRR@5 = 0,6892**, avec 29 chunks de 300 tokens
et chevauchement de 50. Les questions manquées sont `q01`, `q05` et `q12` ; leurs pages
attendues n’apparaissaient pas dans les cinq premiers résultats. Ces échecs de recherche
restent visibles et n’ont pas été retirés du jeu.

La génération finale a été vérifiée sur une question positive (tuple, citation de la
page 10) et une négative (Bitcoin, abstention sans citation), avec le GGUF officiel
importé dans Ollama sous `qwen2.5-local:3b`. Cela valide le fonctionnement de la chaîne,
sans mesurer la qualité du LLM sur l’ensemble du jeu. Les 12 tests locaux passent,
y compris le contrôle des annotations dans le vrai PDF. Windows n’a pas été exécuté
sur cette machine ; la matrice CI prévue permettra de le vérifier après publication
du code sur GitHub.
