#!/bin/bash

# Script d'installation pour BlockBlast
set -e

if [ "$EUID" -ne 0 ]; then
  echo "Veuillez exécuter ce script en tant que root (sudo)."
  exit 1
fi

# Variables
REPO_URL="https://github.com/Mathildeuh/BlockBlast.git"
INSTALL_DIR="/opt/blockblast"
BIN_NAME="BlockBlast"

# Créer un dossier temporaire pour le clone
TMP_DIR=$(mktemp -d)
echo "Clonage du dépôt dans $TMP_DIR ..."
git clone "$REPO_URL" "$TMP_DIR"

cd "$TMP_DIR"

# Compilation
make

# Création du dossier d'installation
mkdir -p "$INSTALL_DIR"
cp "$BIN_NAME" "$INSTALL_DIR/"
cp -r lang "$INSTALL_DIR/"
cp -r include "$INSTALL_DIR/"
cp -r src "$INSTALL_DIR/"
cp genblock.py "$INSTALL_DIR/" 2>/dev/null || true

# Création du raccourci
ln -sf "$INSTALL_DIR/$BIN_NAME" /usr/local/bin/blockblast

# Nettoyage du dossier temporaire
cd /
rm -rf "$TMP_DIR"

# Message de fin
cat <<EOF

BlockBlast a été installé avec succès !

- Lancez le jeu avec : blockblast
- Les fichiers sont dans : $INSTALL_DIR
- Pour désinstaller, supprimez le dossier $INSTALL_DIR et le lien /usr/local/bin/blockblast

EOF 