# Previously published images

Recipes for tags that are still pulled by some project but are no longer built from this
repository. They are kept so that what is inside a running CI job can be read somewhere.

| Tag   | Base        | Still pulled by                        |
|-------|-------------|----------------------------------------|
| `v6`  | Ubuntu 22.04| fluxion                                |
| `v7`  | Ubuntu 22.04| nobody                                 |
| `v9`  | Ubuntu 24.04| kiwaku, raberu, spy                    |
| `v9b` | Ubuntu 24.04| eve, for its three RISC-V jobs only     |
| `v10` | Ubuntu 25.10| the fleet, until each repository moves to `v11` |

## Reproducibility of these recipes

**Running `docker build` on `v6`, `v7`, `v9` or `v9b` does not reproduce the image it describes.**
Every one of them installs Emscripten with `./emsdk install latest` while `ENV PATH` and
`ENV EMSDK_NODE` spell out a fixed node directory - `node/20.18.0_64bit` for `v9` and `v9b`. A
rebuild today pulls a newer node, those two variables point at a directory that does not exist, and
every wasm job in the resulting image dies on `node: Permission denied`. That is what happened to
`v10` on 2026-08-26, and it is why `basic/Dockerfile` now pins the SDK and resolves node through a
version-free symlink.

`v10` is the exception: it pins the SDK with `--depth 1 --branch ${EMSDK_VERSION}` and reaches node
through a version-free `node/current` symlink, which is what the 2026-08-26 breakage bought. Its
recipe is a copy of `basic/` taken when the base moved to `ubuntu:resolute`, and it still builds.

The practical consequence is that **the four older images cannot be rebuilt**. If one is deleted
from GHCR, it is gone. `v9b` is the one to watch: EVE's RISC-V jobs are pinned to it, and they are
pinned there because RVV does not work on `v10` - so it cannot simply be retired either.

## Origin of these recipes

`v7` is the original Dockerfile, which had been sitting untracked in this repository. `v6`, `v9`
and `v9b` were reconstructed on 2026-08-27 from the published images themselves: an OCI image config
carries a `history` array holding the text of every instruction that built it, which is enough to
rebuild the Dockerfile almost verbatim.

```
curl -s "https://ghcr.io/token?scope=repository:jfalcou/compilers:pull&service=ghcr.io"
curl -s -H "Authorization: Bearer $TOKEN" "https://ghcr.io/v2/jfalcou/compilers/manifests/<tag>"
curl -s -L -H "Authorization: Bearer $TOKEN" "https://ghcr.io/v2/jfalcou/compilers/blobs/<config digest>"
```

What that does *not* recover is anything the build read from its context - there was none here - and
the exact base image digest, only its version label. Both are noted at the top of each file.
