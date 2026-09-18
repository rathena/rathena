# Introduction

## Welcome

Thank you for considering contributing to this project. This project is entirely driven by the community, and we appreciate your effort.

## Ground rules to work on this repo

<!-- TODO: Add more rules when needed. -->

- Please be kind and respect other members of the community.
- Always remember this is a FREE project;
- Mind the [LICENSE](../LICENSE) of the project before doing changes.
- Do not bring copyrighted material into the project.

## Getting started

1. Make sure you have a [GitHub account](https://github.com/signup/free).
2. Next, you will need to [fork roBrowserLegacy](https://help.github.com/articles/fork-a-repo/#fork-an-example-repository) to your account.
3. Before making changes, make sure you [create a new branch](https://help.github.com/articles/creating-and-deleting-branches-within-your-repository/) for your working tree. Never use your **master** branch! :bangbang:
4. After completing your changes, commit and push it to your branch.
5. Now you are ready to [create a Pull Request](https://help.github.com/articles/creating-a-pull-request/) for roBrowserLegacy!

Please refer to the [Getting started](./README.md) on how to set up your environment and start working on it.

## Code Quality

Before submitting a PR, make sure your code passes lint and format checks:

```bash
npm run lint          # Check for ESLint errors
npm run lint:fix      # Auto-fix ESLint errors
npm run format        # Format code with Prettier
npm run format:check  # Check formatting without modifying
npm run ci            # Run both lint and format checks (used in CI)
```

### Code Style Guidelines

- Use ES6 `import`/`export` syntax (no AMD `define()`/`require()`)
- Use `const` and `let` (no `var`)
- Use single quotes, semicolons required
- Use modern JavaScript features: arrow functions, template literals, async/await, ES6 classes (see AGENTS.md for detailed modernization patterns)
- Use path aliases for imports (e.g., `import X from 'Utils/X.js'`) instead of relative paths

### How to file a pull request

That will change on what your contribution is trying to achieve. Are you fixing a bug? Proposing an enhancement? Adding a new functionality?

The nice to have structure for those pull requests is:

1. Add an objective title to it, demonstrating what you want to achieve with it, e.g.: "adding support to effect X on library Z" or "fixing an issue with the packet database"
2. Depending on what you're targeting, attach screenshots with the before and after your changes on the client.

We will have an automated mechanism that will do a basic evaluation on the code being offered via the PR, the bare minimum expected is that the project build is passing.

### How to file a bug report

You can open a bug report on the [issues](https://github.com/MrAntares/roBrowserLegacy/issues) page, giving at the bare minimum
the following information:

> 1. What is the roBrowserLegay git commit you're using?
> 2. What NodeJS, browser and operating system you're using?
> 3. What did you do when found this bug?
> 4. What did you expected to happen instead of the error?

You can attach videos and images to help even more, you can upload the videos on platforms like streamable.com or youtube.com. Just
make sure they are public accessible, focus on showing pertinent information about the bug you're reporting.

### Release process

Maintainers of the project can generate new [releases](https://github.com/MrAntares/roBrowserLegacy/releases) for the project by creating tags on specific master branch commit.
After the tag is created and pushed to the GitHub repo, a workflow will be triggered to generate the release package.

They will decide when and how those tags will be applied and which releases they need to generate.

# Code review process

TODO: Who reviews it and what you can expect from the team.

# Community

roBrowser was started by this [awesome team](https://github.com/vthibault/roBrowser/graphs/contributors) and [we](https://github.com/MrAntares/roBrowserLegacy/graphs/contributors) continue it. We also manually add content from other forks that didn't make it back to the original branch, huge shoutout to them!

> You can (and we encourage you) to chat with us on the official [Discord server](https://discord.com/invite/8JdHwM4Kqm).
