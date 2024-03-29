# Contributing

Firstly, thanks for checking out this document and showing interest in contributing :)

## Bug tickets

You may submit patches to this project by forking this repository and making a pull request.
If you want to become a regular contributor, please contact us on discord (check README for the link).
We appreciate all kinds of contributions, whether it's big or small. We also very enthusiastically
welcome bug tickets and improvement suggestions on our bug tracker!

When filing tickets, please first make sure that a ticket does not already exist about your
topic. If one exists, add to the existing ticket if necessary. If a ticket does not yet exist,
feel free to open up a new one. Please make sure that you file as much information as possible
to make it easier for us to track the problem:

In bug tickets, include at least the following:
* Your operating system name and version
* Any logs generated by the game (preferably from debug build)
* As detailed an explanation of the problem as possible, as concisely as possible. This is really
  important!

For feature tickets, we are currently interested only in certain things (more
detailed list below or in milestones view). Before contributing features, make sure to pop in
to #omf channel on freenode and check if the feature is not yet being developed by someone!

For current status, please see discussion page at https://github.com/omf2097/openomf/discussions/413
and our milestone pages for more details.

## Security issues

If you find a security issue in the code, either make a ticket if it's low priority OR send an email at
katajakasa@gmail.com. It may take us a few days to respond.

## Pull Requests

Before making a pull request, please verify via a normal issue ticket
(or via the discussion system or in discord) that nobody else is yet working on the feature.

Pull requests:
* must not have any build errors
* must be formatted using clang-format
* must be verified using clang-tidy

### PR workflow if you don't have merge rights

1. Fork the code in github and create a new branch for yourself
2. Make some changes and commit them. Give the commit a descriptive name.
3. Push the branch and create a PR. Target the omf2097/openomf project (upstream). 
   Explain what your change, what it does and why it should be merged.
4. Other developers will go through your PR, and give comments about it. You should check these
   comments and address them. It is fine to discuss the comments :)
5. When all comments are addressed and the code seems fine and you have some approvals, 
   someone with a merge privileges will show up and Punch The Button.
6. Profit!

### PR workflow if you DO have merge rights

1. Make some changes and commit them. Give the commit a descriptive name.
2. Create a PR and explain the change (either on the PR or on discord).
3. If the commit is small or trivial and you feel confident, just merge it.
   If it's a monster, try poking at other devs and ask for some opinions.
4. Preferably, other devs should leave some comments (and if things go well, approvals).
5. When you feel like it, merge your PR. PR's should be merged by the owner
   of the PR, unless PR has been abandoned.
6. Profit!

Note that we only give merge rights if you have been involved for a while
and we feel you can be trusted. Therefore we also trust you to know how to do
this and when to bend the rules and when not to.
