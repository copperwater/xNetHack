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
3. Merge `master` into `5.0-savebreaking`. Resolve any conflicts.
4. Merge the current vanilla code into `5.0-savebreaking` (or an appropriate
   prior commit if the bleeding-edge commits aren't considered in a good state
   to merge, for instance if the devteam is in the middle of adding a new
   feature or sorting out bugs with a recently added feature it might be better
   to merge up to a commit from before that feature was added). Resolve any
   conflicts.
5. Do the traditional commit that adds saved-up flavor text strings (bogusmons,
   etc.)
6. Build a graphical version and test it out, making sure the tiles are correct.
7. Do any other testing necessary, of the vanilla merge or just in general, such
   as testing using the debug fuzzer or testing part or all of a playthrough.
8. Push the `5.0-savebreaking` branch and confirm that all the windows/mac/Linux
   checks pass github CI. If they don't, fix them with additional commits. Do
   not proceed if any build is failing.
9. Merge `5.0-savebreaking` into `master`. There shouldn't be any conflicts
   unless more commits were added to `master`.
10. Update `VERSION_MAJOR`, `VERSION_MINOR`, `PATCHLEVEL` and
    `VERSION_COMPATIBILITY` in patchlevel.h. Reset `EDITLEVEL` to 0 if it isn't
    already. Update `NH_DEVEL_STATUS` to `NH_STATUS_RELEASED`. Update
    `XNETHACK_VERSION` in the following files (blargh):
    - sys/windows/Makefile.nmake
    - sys/windows/vs/NetHackProperties.props
    - sys/windows/vs/package/package.nmake
    - sys/windows/vs/FetchPrereq/fetchprereq.nmake
    Commit these changes.
11. Push to master. Confirm the CI passes, again.
12. Work with K2 to get the new version working on Hardfought. If there were
    changes to the tileset, make sure the web terminal tiles have also been
    updated. If there are any issues with building on Hardfought, fix them.
13. Tag the patchlevel commit or the most recent fix, e.g. `xnh5.0`, via `git
    tag -a xnh5.0`. Write the tag message using previous release tags as an
    example (xnh6.0 is a good matter-of-fact one). Mention that Windows binaries
    are available to download on the Github release page.
14. Push the tag to master. After several minutes, confirm that there is a new
    "release" on Github with the tag information.
15. There will probably be bugs discovered as players play the new version;
    those should be committed to master so long as the fix doesn't require a
    savebreak.
16. Update the xNetHack wiki page with all the recent changes. Update
    Template:News with the release announcement. Update the xNetHack featured
    article blurb if it is one of the 12 featured article blurbs.
17. Announce new version on Reddit and in IRC.
18. Lay the groundwork for the next release. On the master branch, create an
    empty changelog for the next version, and set `NH_DEVEL_STATUS` to
    `NH_STATUS_POSTRELEASE`.
19. Create a branch off master named "X.0-savebreaking" where X is the next
    version. In that branch, update patchlevel to use the new version numbers
    and set `NH_DEVEL_STATUS` to `NH_STATUS_WIP`.
