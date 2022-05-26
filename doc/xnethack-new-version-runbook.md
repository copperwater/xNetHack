## xNetHack New Version Runbook

This documents the process of releasing a new major version of xNetHack.

Does not currently cover minor versions. They follow a similar process but with
fewer steps (usually triggered by merging a savebreak from vanilla or an
important save-breaking xNetHack feature out for junethack; they don't usually
have release announcements). When the next minor version release happens, this
document may be updated for it.

Assumed premises:
- All work slated for the new version has been completed.
- If there are save-breaking changes, they are present not on the master branch
  but on some other branch. For illustration, let's assume we want to release
  xNetHack 5.0; non-save-breaking commits are already on the `master` branch and
  save-breaking commits are on a branch named `5.0-savebreaking`.

1. Review any open issues and pull requests for changes that should be made
   prior to releasing the version; if there are any, do those first.
2. Review known bugs; if there are any of great enough importance, fix those
   first.
3. Build a graphical version and test it out, making sure the tiles are correct.
4. Do the traditional commit that adds saved-up flavor text strings (bogusmons,
   etc.)
5. Merge `master` into `5.0-savebreaking`. Resolve any conflicts.
6. Merge the current vanilla code into `5.0-savebreaking` (or an appropriate
   prior commit if the bleeding-edge commits aren't considered in a good state
   to merge, for instance if the devteam is in the middle of adding a new
   feature or sorting out bugs with a recently added feature it might be better
   to merge up to a commit from before that feature was added). Resolve any
   conflicts.
7. Push the `5.0-savebreaking` branch and confirm that all the windows/mac/Linux
   checks pass github CI. If they don't, fix them with additional commits. Do
   not proceed if any build is failing.
8. Merge `5.0-savebreaking` into `master`. There shouldn't be any conflicts
   unless more commits were added to `master`.
9. Update `VERSION_MAJOR`, `VERSION_MINOR`, `PATCHLEVEL` and
   `VERSION_COMPATIBILITY` in patchlevel.h. Reset `EDITLEVEL` to 0 if it isn't
   already. Update `NH_DEVEL_STATUS` to `NH_STATUS_RELEASED` if it isn't already
   (it tends to not get changed from that except by the vanilla devteam). Update
   `NETHACK_VERSION` in sys/windows/Makefile.nmake (blargh). Commit these
   changes.
10. Tag the patchlevel commit, e.g. `xnh5.0`, via `git tag -a xnh5.0`. Write the
    tag message using previous release tags as an example (xnh6.0 is a good
    matter-of-fact one). Mention that Windows binaries are available to download
    on the Github release page.
11. Push to master. Confirm the CI passes, again.
12. Push the tag to master. After several minutes, confirm that there is a new
    "release" on Github with the tag information.
13. Work with K2 to get the new version working on Hardfought. Make sure the web
    terminal tiles have also been updated.
14. There will probably be bugs discovered as players play the new version;
    those should be committed to master so long as the fix doesn't require a
    savebreak.
15. Update the xNetHack wiki page with all the recent changes. Update
    Template:News with the release announcement. Update the xNetHack featured
    article blurb if it is one of the 12 featured article blurbs.
16. Announce new version on Reddit and in IRC.
