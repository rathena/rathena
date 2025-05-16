#!/bin/bash


# useful to iterate when fixing the release:
# ver=0.2.1 ; ( set -x ; git tag -d v$ver ; git push origin :v$ver ) ; (set -x ; set -e ; tbump --only-patch --non-interactive $ver ; git add -u ; git commit --amend --no-edit ; git tag --annotate --message "v$ver" "v$ver" ; git push -f --tags origin )


function c4_release_create()
{
    ( \
      set -euxo pipefail ; \
      ver=$(_c4_validate_ver $1) ; \
      branch=$(_c4_validate_branch) ; \
      c4_release_bump $ver ; \
      c4_release_commit $ver $branch \
      )
}

function c4_release_redo()
{
    ( \
      set -euxo pipefail ; \
      ver=$(_c4_validate_ver $1) ; \
      branch=$(_c4_validate_branch) ; \
      c4_release_delete $ver ; \
      c4_release_bump $ver ; \
      c4_release_amend $ver $branch \
    )
}

function c4_release_bump()
{
    ( \
      set -euxo pipefail ; \
      ver=$(_c4_validate_ver $1) ; \
      tbump --non-interactive --only-patch $ver \
      )
}

function c4_release_commit()
{
    ( \
      set -euxo pipefail ; \
      ver=$(_c4_validate_ver $1) ; \
      branch=$(_c4_validate_branch) ; \
      tag=v$ver ; \
      git add -u ; \
      git commit -m $tag ; \
      git tag --annotate --message $tag $tag ; \
      )
}

function c4_release_amend()
{
    ( \
      set -euxo pipefail ; \
      ver=$(_c4_validate_ver $1) ; \
      branch=$(_c4_validate_branch) ; \
      tag=v$ver ; \
      git add -u ; \
      git commit --amend -m $tag ; \
      git tag --annotate --message $tag $tag ; \
    )
}

function c4_release_delete()
{
    ( \
      set -euxo pipefail ; \
      ver=$(_c4_validate_ver $1) ; \
      git tag -d v$ver ; \
      git push origin :v$ver \
    )
}

function c4_release_push()
{
    ( \
      set -euxo pipefail ; \
      ver=$(_c4_validate_ver $1) ; \
      branch=$(_c4_validate_branch) ; \
      tag=v$ver ; \
      git push origin $branch ; \
      git push --tags origin $tag \
      )
}

function c4_release_force_push()
{
    ( \
      set -euxo pipefail ; \
      ver=$(_c4_validate_ver $1) ; \
      branch=$(_c4_validate_branch) ; \
      tag=v$ver ; \
      git push -f origin $branch ; \
      git push -f --tags origin $tag \
    )
}

function _c4_validate_ver()
{
    ver=$1
    if [ -z "$ver" ] ; then \
        exit 1
    fi
    ver=$(echo $ver | sed "s:v\(.*\):\1:")
    #sver=$(echo $ver | sed "s:\([0-9]*\.[0-9]*\..[0-9]*\).*:\1:")
    if [ ! -f changelog/$ver.md ] ; then \
        if [ -f changelog/current.md ] ; then
            git mv changelog/current.md changelog/$ver.md
            touch changelog/current.md
            git add changelog/current.md
        else
            echo "ERROR: could not find changelog/$ver.md or changelog/current.md"
            exit 1
        fi
    fi
    echo $ver
}

function _c4_validate_branch()
{
    branch=$(git rev-parse --abbrev-ref HEAD)
    if [ "$branch" != "master" ] ; then
        echo "ERROR: release branch must be master"
        exit 1
    fi
    echo $branch
}
