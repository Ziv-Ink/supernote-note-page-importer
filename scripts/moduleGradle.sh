#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
exec "$project_root/android/gradlew" --init-script "$project_root/scripts/moduleAnalysis.gradle" "$@"
