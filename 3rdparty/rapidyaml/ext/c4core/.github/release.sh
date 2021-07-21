#!/bin/bash


function c4_release_create()
{
    ( \
      set -euxo pipefail ; \
      ver=$(_c4_validate_ver $1) ; \
      branch=${2:-$(git rev-parse --abbrev-ref HEAD)} ; \
      c4_release_bump $ver ; \
      c4_release_commit $ver $branch \
      )
}

function c4_release_redo()
{
    ( \
      set -euxo pipefail ; \
      ver=$(_c4_validate_ver $1) ; \
      branch=${2:-$(git rev-parse --abbrev-ref HEAD)} ; \
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
      branch=${2:-$(git rev-parse --abbrev-ref HEAD)} ; \
      tag=v$ver ; \
      git add -u ; \
      git commit -m $tag ; \
      git tag --annotate --message $tag $tag ; \
      git push origin $branch ; \
      git push --tags origin $tag \
      )
}

function c4_release_amend()
{
    ( \
      set -euxo pipefail ; \
      ver=$(_c4_validate_ver $1) ; \
      branch=${2:-$(git rev-parse --abbrev-ref HEAD)} ; \
      tag=v$ver ; \
      git add -u ; \
      git commit --amend -m $tag ; \
      git tag --annotate --message $tag $tag ; \
      git push -f origin $branch ; \
      git push -f --tags origin $tag \
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

function _c4_validate_ver()
{
    ver=$1
    if [ -z "$ver" ] ; then \
        exit 1
    fi
    ver=$(echo $ver | sed "s:v\(.*\):\1:")
    #sver=$(echo $ver | sed "s:\([0-9]*\.[0-9]*\..[0-9]*\).*:\1:")
    if [ ! -f changelog/$ver.md ] ; then \
        echo "ERROR: could not find changelog/$ver.md"
        exit 1
    fi
    echo $ver
}
