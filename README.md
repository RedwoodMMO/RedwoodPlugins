# Redwood Plugins

This repository contains all of the core plugins to work with the Redwood backend.

## Royalty EULA

Please read the [license](./LICENSE.md) fully. By using any of the source from this repository in your project, with or without the Redwood backend, you're agreeing to the Redwood End User License Agreement. The Redwood EULA has revenue royalty obligations which you may be subject to by using any source from this repository.

## Get Started

To get started with Redwood, [register for a Redwood account](https://license.redwoodmmo.com) and follow our [getting started documentation](https://redwoodmmo.com/docs/getting-started/overview).

The Redwood backend's source code is only available for users with custom license, but you can download a precompiled version by following the above instructions.

## Downloading This Repository

⚠️ **You cannot use GitHub's "Download ZIP" option!** ⚠️

This repository containers submodules (that are recursive) and LFS data, and GitHub's ZIP functionality will not pull these for you.

### Using git

You can clone this repository recursively:

```bash
git clone --recurse-submodules https://github.com/RedwoodMMO/RedwoodPlugins.git
```

If you forgot to clone recursively, you can still initialize the submodules recursively

```bash
git submodule update --init --recursive
```

If you need to pull the most recent changes, make sure you have the recurse-submodules option:

```bash
git pull --recurse-submodules
```

### Downloading a release ZIP

We package the source into a downloadable ZIP in each of our releases. Find the `RedwoodPlugins-x.y.z.zip` in the [latest release](https://github.com/RedwoodMMO/RedwoodPlugins/releases/latest).
