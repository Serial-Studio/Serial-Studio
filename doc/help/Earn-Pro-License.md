# Earn a Pro License

The free edition already plots `Serial.println` values over USB serial. Serial Studio Pro adds MQTT, Modbus, CAN Bus and the 3D plot (see [Pro vs Free](Pro-vs-Free.md)). If you need those and can't buy a license, you can earn one: translate a page, write a tutorial or get a fix merged, and the maintainer issues a Serial Studio Pro license key for a fixed period. One developer maintains Serial Studio, so contributions pay for a license.

To try Pro first, use any Pro feature, for example MQTT, and answer **Yes** to the prompt "Start your free 14-day Serial Studio Pro trial?". The trial is offered, never started automatically, and registering it needs an internet connection. After it expires, the prompt offers activation instead.

## What you get

A contribution earns a time-limited Pro license key, not a perpetual one. The maintainer picks the duration within the range in the table below, based on the size, quality and reach of the work. Contributions must be your own work and checkable by the maintainer: send links, files, screenshots or a pull request. Durations are not guaranteed; the maintainer confirms yours in the Discussion reply.

| Contribution type              | Examples                                                                   | Typical duration |
|--------------------------------|----------------------------------------------------------------------------|------------------|
| Forum post, tutorial, or video | Reddit, CSDN, Hackaday, YouTube, Bilibili                                  | 6 months         |
| Social media or community post | A post with proof of reach (engagement metrics, screenshots)               | Set by the maintainer |
| Hardware donation              | Boards, analyzers, adapters                                                | 12 months        |
| Protocol log or sample data    | A capture of the bytes an unsupported device sends, so Serial Studio can learn to parse it | 3-6 months |
| Translation or documentation   | Translating the UI or help pages, writing or improving docs                | 6-12 months      |
| Code contribution (merged PR)  | Bug fixes, new features, platform support                                  | 6-12 months      |

Serial Studio Pro activates an earned key like a purchased one: click **About**, then **Manage License**, paste the key and click **Activate**. Activation needs an internet connection. See [Upgrading from Free to Pro](Pro-vs-Free.md#upgrading-from-free-to-pro).

## How to submit

1. Open a [GitHub Discussion](https://github.com/Serial-Studio/Serial-Studio/discussions) with the `value-for-license` tag. If you can't apply the tag, the maintainer adds it.
   - Title: `Request: Pro License via [Contribution Type]`, using a type from the table in [What you get](#what-you-get). For example: `Request: Pro License via Translation (French)`.
   - Describe what you did (links, files, screenshots).
   - Discussions are public, so don't post your email address there. Say you'll share it when asked.
2. The maintainer replies in the Discussion, usually within a few days, with no guaranteed schedule.
3. An approved request gets a license key by email.

## Getting started with code contributions

Code contributions are mostly C++, Qt and QML; translations and documentation need none of these.

1. Fork the repository on GitHub.
2. Read [CONTRIBUTING.md](https://github.com/Serial-Studio/Serial-Studio/blob/master/CONTRIBUTING.md), which covers the contribution rules and code style.
3. Pick an open issue or propose a new feature.
4. Submit your pull request. Contributions are accepted under the [Contributor License Agreement](License-Agreement.md#contributor-license-agreement-cla), described in CONTRIBUTING.md.
5. Once it's merged, request your license as described in [How to submit](#how-to-submit).

## See also

- [Pro vs Free](Pro-vs-Free.md): what Pro unlocks.
- [License Agreement](License-Agreement.md): dual-license details.
- [Support Serial Studio](Support-Serial-Studio.md): other ways to help.
- [Become an Affiliate](Become-an-Affiliate.md): earn commission by referring customers, if you have an audience.
- [Buy Serial Studio Pro](https://store.serial-studio.com/): purchase a Pro license.
