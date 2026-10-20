# RAG Python local — Windows et Linux

Projet de recherche documentaire sur PDF : extraction avec PyMuPDF, découpage en
tokens, embeddings **Sentence Transformers**, index **FAISS**, réponses avec
**Ollama**, citations document/page et évaluation de la recherche.

Corpus gratuit : le glossaire par chapitre de *Think Python* d’Allen B. Downey
(**15 pages en anglais**), téléchargé depuis une révision fixe du dépôt officiel.
Les questions peuvent être françaises ou anglaises. [Source et droits](corpus/README.md).
Le corpus dérive d’un livre sous CC BY-NC 3.0 : usage non commercial.

## Prérequis

- **Windows 10/11 x64** ou **Linux x86_64** récent (glibc >= 2.28), Python **3.11 ou 3.12**.
- [uv](https://docs.astral.sh/uv/getting-started/installation/) pour l’installation verrouillée.
- [Ollama](https://ollama.com/download) installé avec l’installeur natif de votre OS.
- CPU suffisant ; GPU facultatif pour Ollama. Prévoir **8 Go de RAM** au minimum
  (16 Go conseillés) et environ **10 Go de disque libre** selon les caches/installateurs.
- Connexion Internet pour les premiers téléchargements uniquement. Pas de clé API.

Modèles par défaut :

| Rôle | Modèle | Choix |
|---|---|---|
| Embeddings | `intfloat/multilingual-e5-small` via Sentence Transformers | Multilingue, 384 dimensions, CPU ; révision Hugging Face fixée |
| Génération | `qwen2.5:3b` dans Ollama | Français/anglais, modèle compact d’environ 1,9 Go quantifié |

Le tag Ollama est importé explicitement ; sa version exacte peut évoluer en amont.
Consulter `ollama list` pour son identifiant et `ollama show qwen2.5:3b` pour ses
métadonnées et sa licence. Les modèles ont leurs propres conditions d’utilisation.

## Installation

Toutes les commandes suivantes s’exécutent **à la racine du dépôt**, sur la branche
`preparation-projet`. Aucun droit administrateur n’est nécessaire pour le projet Python.

```sh
uv sync --frozen --extra dev
```

`uv.lock` fixe les dépendances et les empreintes des paquets pour les deux OS et les
deux versions de Python. PyTorch est pris dans l’index CPU officiel, sans pile CUDA.
`uv run --frozen` utilise automatiquement `.venv` : aucune activation PowerShell
ni modification d’ExecutionPolicy n’est nécessaire.

Alternative sans uv (résolution non verrouillée) : créer un venv, l’activer puis :

```sh
python -m pip install torch --index-url https://download.pytorch.org/whl/cpu
python -m pip install -e ".[dev]"
```

Sous Linux sans installation système d’Ollama, le helper optionnel suivant installe
la version **0.40.5** dans un répertoire local, avec SHA-256 officiel fixé. Il requiert
`curl`, `tar`, `zstd` et `sha256sum` :

```sh
bash scripts/install_ollama_linux.sh
```

Dans ce cas, remplacer `ollama` par `.cache/ollama/bin/ollama` dans les commandes de
démarrage. Sous Windows, utiliser l’installeur officiel et rouvrir le terminal.

## Exécution de bout en bout

Démarrer Ollama dans un **autre terminal** si son application/service ne tourne pas :

```sh
ollama serve
```

Puis, depuis la racine du dépôt (commandes identiques sous PowerShell et Bash) :

```sh
uv run --frozen rag download
uv run --frozen rag extract
uv run --frozen rag index
uv run --frozen rag pull-model
uv run --frozen rag search "Qu’est-ce que la récursion ?" -k 5
uv run --frozen rag ask "Qu’est-ce que la récursion ?" -k 5
uv run --frozen rag evaluate -k 5
uv run --frozen python -m pytest -q
```

`index` refait lui-même l’extraction ; `extract` est fourni pour examiner les pages.
La première indexation télécharge les poids E5 (environ 470 Mo). `pull-model` importe
le LLM par l’API locale Ollama ; l’équivalent natif est `ollama pull qwen2.5:3b`.
Sans Ollama, téléchargement, extraction, indexation, recherche et évaluation restent
utilisables. Seules l’importation du LLM et la génération nécessitent son service.

`ask` demande une sortie JSON contrainte à Ollama, avec la réponse, les identifiants
des sources et un indicateur `insufficient_context`. Le résultat contient aussi les
extraits, scores, pages et repères `[S1]`, `[S2]`, etc. Les références sélectionnées
par le modèle sont rendues visibles dans la réponse. Les citations absentes/invalides sont signalées ; une
citation syntaxiquement valide ne garantit pas que l’affirmation est correcte.
Le prompt demande de s’abstenir si les documents ne suffisent pas. Les instructions
présentes dans les PDF sont traitées comme des données, mais la résistance à
l’injection de prompt n’est pas garantie.

Pour choisir un autre serveur ou modèle :

```sh
uv run --frozen rag pull-model --llm qwen2.5:7b --host http://127.0.0.1:11434
uv run --frozen rag ask "What is a tuple?" --llm qwen2.5:7b
```

`OLLAMA_HOST` est aussi accepté (URL HTTP/HTTPS complète). Un modèle plus grand
nécessite davantage de mémoire. Depuis un autre répertoire, utiliser
`rag --root CHEMIN_DU_DEPOT ...`.

## Import GGUF si le registre Ollama est inaccessible

Une alternative utilise directement le **GGUF officiel Qwen2.5-3B-Instruct Q4_K_M**
sur Hugging Face (environ 2,1 Go), puis les API publiques d’import Ollama. Elle ne
nécessite aucun fichier Modelfile ni accès du serveur Ollama à Internet :

```sh
uv run --frozen python scripts/download_gguf.py
uv run --frozen rag import-gguf artifacts/llm/qwen2.5-3b-instruct-q4_k_m.gguf --llm qwen2.5-local:3b
uv run --frozen rag ask "Qu’est-ce que la récursion ?" --llm qwen2.5-local:3b
```

Le script fixe la révision officielle
`Qwen/Qwen2.5-3B-Instruct-GGUF@7dabda4d13d513e3e842b20f0d435c732f172cbe`
et vérifie le SHA-256 publié par le stockage Hugging Face avant l’import. Le fichier
reste hors Git. Le modèle importé porte un nom distinct du tag du registre.
`import-gguf` accepte aussi vos propres GGUF compatibles Ollama, à condition d’avoir
vérifié leur provenance et leurs droits. Ollama déduit le template du GGUF ; les
instructions documentaires sont transmises à chaque requête.

## Fonctionnement et fichiers

```text
PDF vérifié → pages avec provenance → chunks de 300 tokens, chevauchement 50
            → E5 : préfixe passage:, vecteurs normalisés → FAISS IndexFlatIP
Question    → E5 : préfixe query: → top K → Ollama → réponse avec sources
```

Le découpage respecte les pages et le tokenizer réel. La taille reste sous la limite
E5 de 512 tokens, en réservant la place des préfixes et tokens spéciaux. FAISS réalise
une recherche exacte par produit scalaire sur des vecteurs normalisés, soit le
cosinus ; ce choix simple convient à un petit corpus. Aucun entraînement n’est requis.

- `corpus/manifest.json` : sources, droits, empreintes et pagination.
- `src/rag_python/` : CLI, téléchargement, extraction, index, évaluation et client Ollama.
- `evaluation/questions.jsonl` : **20 questions positives + 3 hors corpus**, réponses et preuves.
- `data/pdfs/`, `data/pages.jsonl` : documents téléchargés et extraction, ignorés par Git.
- `artifacts/index/generation-*/` : index FAISS, chunks JSONL et métadonnées.
- `artifacts/index/CURRENT` : génération active ; remplacement atomique après écriture.
- `artifacts/evaluation.json` : rapport de recherche, rangs et scores par question.
- `.cache/huggingface/` : poids d’embeddings, ignorés par Git.

Les anciennes générations sont conservées pour permettre un retour manuel en arrière.
La révision réellement chargée du modèle est enregistrée et réutilisée à la recherche.
Il faut reconstruire l’index après changement de corpus, modèle ou découpage :

```sh
uv run --frozen rag index --chunk-size 250 --overlap 40 --batch-size 8
```

Les modèles alternatifs doivent suivre le protocole E5 (`query:`/`passage:`), et
`--revision` peut fixer leur commit. Ne charger que des index locaux de confiance :
les empreintes détectent la corruption accidentelle, sans signer les fichiers.

## Évaluation et limites

[Protocole détaillé](evaluation/README.md) : **Hit@K**, **MRR@K**, et rappel des preuves.
Les questions et réponses de référence ne sont jamais indexées ni données au LLM.
Les négatifs servent à examiner l’abstention ; ils ne sont pas inclus dans les scores
de recherche. L’évaluation automatique ne juge pas les réponses générées.

Les tests courants utilisent de vrais PDF et FAISS, avec un encodeur déterministe
pour tester la mécanique sans télécharger de modèle. La vérification des annotations
sur le corpus réel est ignorée explicitement si `rag download` n’a pas été exécuté.
La CI fournie prévoit Linux et Windows, Python 3.11/3.12 ; elle ne prouve pas une
exécution Windows tant qu’un run sur ce système n’a pas abouti. Un lancement manuel
`workflow_dispatch` ajoute une indexation E5 réelle et conserve le rapport de recherche.

## Dépannage

- **Ollama inaccessible** : démarrer `ollama serve`, vérifier `ollama list`, puis
  `rag pull-model`. Si le port est occupé, utiliser le service déjà présent.
- **Mémoire insuffisante** : réduire `--batch-size` pour E5 ; choisir un LLM plus petit
  avec `--llm` pour Ollama. Les embeddings fonctionnent sur CPU par défaut.
- **PDF modifié** : le SHA-256 bloque un téléchargement différent. Examiner la source
  avant de modifier le manifeste ; ne pas supprimer cette vérification.
- **PDF scanné** : ajouter un OCR préalable. Pas d’OCR inclus.
- **Réseau filtré** : autoriser GitHub/raw.githubusercontent.com, PyPI/files.pythonhosted.org,
  download.pytorch.org et ses redirections, huggingface.co, us.aws.cdn.hf.co,
  cas-bridge.xethub.hf.co, registry.ollama.ai et le stockage R2 indiqué par Ollama.
  Ne pas désactiver TLS. Le client utilise HTTP standard Hugging Face par défaut
  (`HF_HUB_DISABLE_XET=1`) et respecte les proxies de l’environnement.
- **Cache** : `HF_HOME` peut personnaliser le cache ; sinon le projet utilise
  `.cache/huggingface`. Une fois les modèles présents, la recherche peut être lancée
  hors ligne avec `HF_HUB_OFFLINE=1` ; Ollama doit toujours être démarré pour `ask`.
