How to make an OpenOMF release
==============================

On 0.7.x
--------

For 0.7.x releases, use the releases/0.7.x branch. You need to edit the version in `.github/workflows/compilation.yml` and `CMakeLists.txt`.

You also need to add a new `<release>` to the `resources/flatpak/org.openomf.OpenOMF.metainfo.xml` file.

An example PR is here: https://github.com/omf2097/openomf/pull/863 .

Once that PR is merged, make an annotated tag on the merge commit of the form 0.7.<MINOR> where MINOR is your new minor version.

If needed, also update the flathub manifest at https://github.com/flathub/org.openomf.OpenOMF .

0.8.x and above
---------------

Refer to this PR: https://github.com/omf2097/openomf/pull/840

You should update the `resources/flatpak/org.openomf.OpenOMF.metainfo.xml` as noted above in a PR.

Once that PR is merged, make an annotated tag on the merge commit for the version you're tagging. Versions must be semver compatible.

As above, update the flathub manifest if needed.
