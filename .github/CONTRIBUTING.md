# Contributing to rAthena


Table of Contents
-----------------

  * [Reporting Bugs](#reporting-bugs)
  * [Suggesting Enhancements](#suggesting-enhancements)
  * [Issue Labels](#issue-labels)
  * [Become a Team Member](#become-a-team-member)

Reporting Bugs
--------------

This bit of information is here to guide you through the process of creating a bug report for rAthena! Issues cannot only be used for developers to track bugs, but they can also track enhancements and tasks. The more detailed your report is, the easier it is for developers to reproduce and resolve the bug!

### Did you find a bug? :bug:

* **Ensure the bug is not coming from a customization** within your files!
* **Ensure the bug was not already reported** by searching on GitHub under [Issues](https://github.com/rathena/rathena/issues). If the same issue exists, feel free to leave a comment and tell us you are experiencing the issue as well and if possible add some additional or missing information!
* If you are unable to find an open issue addressing the problem, [open a new one](#submit-a-bug-report)!

#### Submit A Bug Report :inbox_tray:

There are several things that go into making a bug report a good bug report!

This is a breakdown of a generic Issue:
* **Title** should give some general insight as to what the bug is about.
* **Description** should give greater detail of the bug that cannot be explained in the **Title**.
* **Labels** are colored to represent a category they fall into.
* **Milestones** are what developers use to group tasks together and quickly evaluate how close the project deliverable is near completion.
* **Assignees** are developers who are directly linked to resolve the issue.
* **Comments** allow other members to give feedback on the issue.

#### What are some good details to provide in a bug report? :pencil2:

When describing your Issue through the **Description** area, it is recommended that you provide as much information as possible to resolve the Issue as fast as possible. Keep in mind that you can tag people within the **Description** area through the `@mention` feature. You can also tag other Issues or Pull Requests by typing `#` which will pull up a list of issues. You can find a markdown guide at [Mastering Markdown](https://guides.github.com/features/mastering-markdown/).
Some information to keep in mind while creating an Issue:
* **GitHub Hash**: The hash is a 40 alpha-numeric string (which can be broken down to the first 7 characters) which states the version you are at. (**If you are using SVN instead of Git:** Please also put the change date and first line of the commit message beside the revision number, or we will not be able to look up the corresponding Git hash).
* **Client Date**: The client date provides specific details depending on the issue. The main detail is that it helps narrow down issues that are related to a packet problem.
* **Modifications that may affect results**: It is always best to try to reproduce your issue on a clean rAthena if you have lots of modifications.
* **Description of Issue**: Describe your issue in detail! Screenshots and videos help a lot! Please also provide crash dumps if one of the servers is crashing.
* **How to Reproduce Issue**: Describe how to reproduce your issue in detail! The more the merrier!
* **Official Info**: Provide creditable sources to state why it is a bug! Please do not provide an iRO Wiki link as there is a high chance it does not match kRO behavior.

#### Be wary of the `@mention` feature! :warning:

Since rAthena uses custom `@commands`, when describing an issue that deals with these commands please keep in mind that this does clash with the `@mention` system for GitHub! Always quote the text when mentioning an ``` `@command` ```(like this) so that you do not tag uninvolved GitHub users!

Suggesting Enhancements
-----------------------

### Did you write a patch that fixes a bug? :bookmark_tabs:

* Open a new GitHub Pull Request with the patch.
* Ensure the PR description clearly describes the problem and solution. Include the relevant issue number if applicable.

### Do you intend to add a new feature or change an existing one? :bulb:

* Open a new GitHub Pull Request with the feature addition or changes.
* Ensure the PR description clearly describes what the addition or changes are for. Include the relevant issue number if applicable.

#### How to create Pull Requests :pencil:

1. Make sure you have a [GitHub account](https://github.com/signup/free).
2. Next, you will need to [fork rAthena](https://help.github.com/articles/fork-a-repo/#fork-an-example-repository) to your account.
3. Before making changes, make sure you [create a new branch](https://help.github.com/articles/creating-and-deleting-branches-within-your-repository/) for your working tree.
4. After completing your changes, commit and push it to your branch.
5. Now you are ready to [create a Pull Request](https://help.github.com/articles/creating-a-pull-request/) for rAthena!
  * Upon creating the Pull Request, make sure you follow our [template](https://github.com/rathena/rathena/blob/master/.github/PULL_REQUEST_TEMPLATE.md) and provide the required information.
  * **OPTIONAL**: We would greatly appreciate those that check the box to [allow edits by maintainers](https://help.github.com/articles/allowing-changes-to-a-pull-request-branch-created-from-a-fork/), so we can apply small cleanups or additions to your changes before merging them!

Issue Labels
------------

For the most part you as a user will have no reason to worry about the **Milestone** or **Assignee** parts of an Issue. The different **Labels** of an Issue allow developers to quickly understand the issue and also allows for fast searching or sorting.

:bangbang: Users should be aware of the 'Mode' and 'Status' Type **Labels** as these sometimes require feedback! :bangbang:

#### Component

| Label Name | Search Link | Description |
| --- | --- | --- |
| `component:core` | [search][search-rathena-label-componentcore] | A fault that lies within the main framework of rAthena. |
| `component:database` | [search][search-rathena-label-componentdatabase] | A fault that lies within the database of rAthena. |
| `component:documentation` | [search][search-rathena-label-componentdocumentation] | A fault that lies within the documentation of rAthena. |
| `component:script` | [search][search-rathena-label-componentscript] | A fault that lies within the scripts of rAthena. |
| `component:skill` | [search][search-rathena-label-componentskill] | A fault that deals specifically with a skill. |
| `component:tool` | [search][search-rathena-label-componenttool] | A fault that lies within a tool of rAthena. |

#### Missing
| Label Name | Search Link | Description |
| --- | --- | --- |
| `missing:clientdate` | [search][search-rathena-label-missingclientdate] | Issue **Title** or **Description** does not state the client date used to create the bug. |
| `missing:mode` | [search][search-rathena-label-missingmode] | Issue **Title** or **Description** does not state pre-renewal or renewal mode. |
| `missing:revision` | [search][search-rathena-label-missingrevision] | Issue **Description** does not state the revision of rAthena used when the bug occurred. |

#### Mode
| Label Name | Search Link | Description |
| --- | --- | --- |
| `mode:prerenewal` | [search][search-rathena-label-modeprerenewal] | A fault that exists within the pre-renewal mode. |
| `mode:renewal` | [search][search-rathena-label-moderenewal] | A fault that exists within the renewal mode. |

#### Priority

| Label Name | Search Link | Description |
| --- | --- | --- |
| `priority:high` | [search][search-rathena-label-priorityhigh] | A fault that makes rAthena unstable or unusable. |
| `priority:medium` | [search][search-rathena-label-prioritymedium] | A fault that makes rAthena have significant repercussions but does not render rAthena unusable. |
| `priority:low` | [search][search-rathena-label-prioritylow] | A fault that affects rAthena in one piece of functionality and is self-contained. |

#### Status

| Label Name | Search Link | Description |
| --- | --- | --- |
| `status:code-review` | [search][search-rathena-label-statuscodereview] | Pull Request that requires reviewing from other developers before being pushed to master. |
| `status:confirmed` | [search][search-rathena-label-statusconfirmed] | Issue that has been validated by a developer to affect rAthena. |
| `status:duplicate` | [search][search-rathena-label-statusduplicate] |  Issue that has been reported before. |
| `status:inprogress` | [search][search-rathena-label-statusinprogress] | Issue that has begun resolution by a developer. |
| `status:invalid` | [search][search-rathena-label-statusinvalid] | Issue that is either not official or is not related to rAthena. |
| `status:need more info` | [search][search-rathena-label-statusneedmoreinfo] | Issue that needs more information from a creditable source. |
| `status:need user input` | [search][search-rathena-label-statusneeduserinput] | Issue that needs more information from the issue creator. |
| `status:outdated emulator` | [search][search-rathena-label-statusoutdatedemulator] | Issue that requires the creator's local files to be updated to be resolved. |
| `status:unable to reproduce` | [search][search-rathena-label-statusunabletoreproduce] | Issue that was unable to be reproduced on rAthena. |
| `status:wontfix` | [search][search-rathena-label-statuswontfix] |  Issue that cannot be fixed through some limitation or is intended behavior. |

#### Type
| Label Name | Search Link | Description |
| --- | --- | --- |
| `type:bug` | [search][search-rathena-label-typebug] | Issue that is a bug within rAthena. |
| `type:enhancement` | [search][search-rathena-label-typeenhancement] | Issue that is an enhancement to rAthena. |
| `type:maintenance` | [search][search-rathena-label-typemaintenance] | Issue for refactoring rAthena. |
| `type:question` | [search][search-rathena-label-typequestion] | Issue that is a question for rAthena. |

[search-rathena-label-componentcore]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Acomponent%3Acore
[search-rathena-label-componentdatabase]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Acomponent%3Adatabase
[search-rathena-label-componentdocumentation]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Acomponent%3Adocumentation
[search-rathena-label-componentscript]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Acomponent%3Ascript
[search-rathena-label-componentskill]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Acomponent%3Askill
[search-rathena-label-componenttool]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Acomponent%3Atool
[search-rathena-label-missingclientdate]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Amissing%3Aclientdate
[search-rathena-label-missingmode]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Amissing%3Amode
[search-rathena-label-missingrevision]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Amissing%3Arevision
[search-rathena-label-modeprerenewal]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Amode%3Aprerenewal
[search-rathena-label-moderenewal]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Amode%3Arenewal
[search-rathena-label-priorityhigh]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Apriority%3Ahigh
[search-rathena-label-prioritymedium]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Apriority%3Amedium
[search-rathena-label-prioritylow]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Apriority%3Alow
[search-rathena-label-statuscodereview]: https://github.com/rathena/rathena/pulls?q=is%3Apr+is%3Aopen+label%3Astatus%3Acode-review
[search-rathena-label-statusconfirmed]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Astatus%3Aconfirmed
[search-rathena-label-statusduplicate]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Astatus%3Aduplicate
[search-rathena-label-statusinprogress]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Astatus%3Ainprogress
[search-rathena-label-statusinvalid]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Astatus%3Ainvalid
[search-rathena-label-statusneedmoreinfo]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3A"status%3Aneed+more+info"
[search-rathena-label-statusneeduserinput]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3A"status%3Aneed+user+input"
[search-rathena-label-statusoutdatedemulator]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3A"status%3Aoutdated+emulator"
[search-rathena-label-statusunabletoreproduce]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3A"status%3Aunable+to+reproduce"
[search-rathena-label-statuswontfix]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Astatus%3Awontfix
[search-rathena-label-typebug]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Atype%3Abug
[search-rathena-label-typeenhancement]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Atype%3Aenhancement
[search-rathena-label-typemaintenance]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Atype%3Amaintenance
[search-rathena-label-typequestion]: https://github.com/rathena/rathena/issues?q=is%3Aissue+is%3Aopen+label%3Atype%3Aquestion

Become a Team Member
--------------------

1. Before you send in a staff application, make sure you have an [rAthena account](https://rathena.org/board/register/).
    * If you are new to the community, go ahead and [introduce yourself](https://rathena.org/board/forum/89-introductions/)!
2. Please fill out the [Staff Application](https://rathena.org/board/staffapplications/) and you will be notified shortly.

The rAthena team is comprised of all volunteers ([AUTHORS](https://github.com/rathena/rathena/blob/master/AUTHORS)). We encourage you to pitch in and submit bug reports or Pull Requests!

Thanks!

rAthena Team
