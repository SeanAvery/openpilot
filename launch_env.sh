#!/usr/bin/bash

export OMP_NUM_THREADS=1
export MKL_NUM_THREADS=1
export NUMEXPR_NUM_THREADS=1
export OPENBLAS_NUM_THREADS=1
export VECLIB_MAXIMUM_THREADS=1

if [ -z "$AGNOS_VERSION" ]; then
  export AGNOS_VERSION="9.3"
fi

export STAGING_ROOT="/data/safe_staging"

TURBO_ENV="/data/etc/turbo.env"
if [ -f $TURBO_ENV ]; then
  while IFS='=' read -r key value || [[ -n "$key" ]]; do
    # Skip comments and empty lines
    [[ "$key" =~ ^\s*#.*$ || -z "$key" ]] && continue
    # Remove leading/trailing whitespace
    key=$(echo "$key" | xargs)
    value=$(echo "$value" | xargs)

    export "$key=$value"
  done < "$TURBO_ENV"
fi
