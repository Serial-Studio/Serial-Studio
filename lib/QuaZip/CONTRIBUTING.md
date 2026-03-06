Fixed a bug? Implemented a new feature? Want to have it included in QuaZip?
Here's what you need to do.

0. If you don't have a GitHub account, create one. It's ridiculously easy.

1. First, open an [issue](https://github.com/stachenov/quazip/issues).
Even if you have already fixed it. It helps to track things because
instead of a commit saying “fixed this and that” you have a reference
to a full issue description.

2. Next, figure out the right branch to work on. *Usually* it's `master`,
but sometimes it's something different. For example, there was time
when new features went to `pre1.0`, and `master` was for 0.x bugfixes only.

3. Next, send a PR (pull request). There are numerous articles how to do it,
but it's not exactly intuitive, just like anything with Git, so do
some research before attempting to create a PR.

**Contribution guidelines**

To avoid spending time on something that may never be accepted, here are
some guidelines.

1. Changes should be backwards-compatible. Don't just change method names
and their signatures randomly. Don't just remove deprecated features—some
of them are there to keep compatibility with old Qt versions. Even Qt 4 is
still supported! Unless you're working on some sort of `pre2.0` branch
or something like that, you should keep ABI compatibility as well! Meaning,
no adding virtual functions, no changing parameter types, even if it's
a source-compatible change, and a lot of other things, really.

2. If your change targets some new version of Qt, it's a very good idea
to keep various `#ifs` in `quazip_qt_compat.h`, which exists for this very
purpose. Don't just throw them into the code. Don't just replace deprecated
things with their newer alternatives—it would break support of the old
Qt versions.

3. If your changes aren't limited to a small fix or a convenience method,
discussing them in the issue you just opened (if you didn't, do!) could
help to achieve some agreement on what exactly is a good idea in your case.

4. Whether you're fixing a bug or adding a new feature, it's an awesome idea
to add a new test in the `qztest` subproject. This way you make sure the
bug stays fixed and the feature keeps working. This is *not* a mandatory
requirement, however, because adding tests to a project you're not familiar
with can be challenging, and it should not stop you from sending a PR.

5. It would be nice if you also update NEWS.txt with whatever changes
you propose. Just add another line on top.

6. QuaZip doesn't really have any policies on PR commits. Just use common sense.
Generally, it's better to keep separate things separate. If you made 2 changes,
and there are 6 commits for one change and 4 for another, that's OK.
Don't squash commits just to squash them. If your branch gets outdated before PR
is accepted, merge or rebase—it doesn't really mater, as long as there are no
conflicts.
