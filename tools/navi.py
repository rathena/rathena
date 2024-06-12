#!/usr/bin/python3

import re


def parse_setarray(s):
    """
    parse setarray
    returns list of x,y tuples
    """
    # D1: setarray @c[2],261,272,275,270,116,27;
    res = re.match(r".*\],(.*);", s)
    # print(res.group(1))

    spl = res.group(1).split(',')
    l = [spl[i:i + 2] for i in range(0, len(spl), 2)]
    # print(l)
    return l


def parse_disp(s):
    """
    parse disp
    returns name for all, or list of names for each
    """
    # Disp("Comodo Field",1,9);
    res = re.match(r""".*"(.*)".*,.*(\d+),(\d+)\).*""", s)
    if res:
        # print(res.group(1))
        # print(res.group(2))
        # print(res.group(3))
        l = [f"{res.group(1)} {i}" for i in range(int(res.group(2)), int(res.group(3)) + 1)]
        # print(l)
        return l

    res = re.match(r'.*Disp\("(.*)"\);.*', s)
    l = res.group(1).split(':')
    # print(l)
    return l


def parse_pick(s):
    """
    parse pick
    returns base fld for all, or list of flds for all
    """
    if s.count('"') > 2:
        # need to return lsit of fields
        res = re.match(r'Pick\("",(.*)\).*', s)
        # print(res.group(1))
        l = res.group(1).split(',')
        l = [m[1:-1] for m in l], 0
        return l
    elif ',' in s:
        # need to subtract number
        res = re.match(r""".*\("(.*)".*(\d+).*\)""", s)
        # print(res.group(1))
        return res.group(1), int(res.group(2))
    else:
        res = re.match(r""".*\("(.*)"\)""", s)
        # print(res.group(1))
        return res.group(1), 0


def split_disp_pick(s):
    """
    returns tuple of <"disp(..)", "pick(...)">
    """

    res = re.match(r""".*(Disp\(.*\);).*(Pick\(.*\);).*""", s)
    # print(res.group(1))
    # print(res.group(2))
    return res.group(1), res.group(2)


def gen_mapname(map, i, diff):
    """
    take man_fild, 3, 0
    output man_fild03

    diff is the subtract for gef_fild00
    """
    if isinstance(map, str):
        i = i + 1 - diff
        return f"{map}{'0' if i < 10 else ''}{i}"
    elif isinstance(map, list):
        return map[i]
    else:
        raise Exception


def main():

    # parse_disp('Disp("Sograt Desert 1:Sograt Desert 2:Sograt Desert 3:Sograt Desert 7:Sograt Desert 11:Sograt Desert 12:Sograt Desert 13:Sograt Desert 16:Sograt Desert 17:Sograt Desert 18:Sograt Desert 19:Sograt Desert 20:Sograt Desert 21:Sograt Desert 22");')
    # parse_setarray(
    #     '	 setarray @c[2],219,205,177,206,194,182,224,170,198,216,156,187,185,263,206,228,208,238,209,223,85,97,207,202,31,195,38,195;')
    # return

    while (1):
        setarray = input("input setarray")
        disppick = input("input disppick")

        poses = parse_setarray(setarray)
        disp, pick = split_disp_pick(disppick)

        names = parse_disp(disp)
        maps, diff = parse_pick(pick)

        for i in range(len(poses)):
            if i == 1:
                print("if (!.OnlyFirstDun) {")
            if i > 0:
                tabs = "    "
            else:
                tabs = ""
            print(
                f'{tabs}naviregisterwarp("Warper > {names[i]}", "{gen_mapname(maps, i, diff)}", {poses[i][0]}, {poses[i][1]});')
            if i > 0 and i == len(poses) - 1:
                print("}")


if __name__ == "__main__":
    main()
