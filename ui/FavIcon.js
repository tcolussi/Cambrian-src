.pragma library

function host(url) {
    var idx = url.search("://");
    var s = url.substring(idx + 3);
    var idx2 = s.search("/");
    return url.substring(0, idx + idx2 + 3);
}

/*
    Returns the URL of a fav icon in the given HTML data, or an empty string if
    none was found.
*/

function favIcon(data) {
    var idx1 = data.search(/<link .*rel *=.*shortcut icon/i);
    var idx2 = data.search(/<link .*rel *=.*icon/i);
    if (idx1 === -1 && idx2 === -1)
        return "";

    var s = data.substring(idx1 === -1 ? idx2 : idx1);
    var tmp = s.search(">");
    s = s.substring(0, tmp);

    idx1 = s.search("href");
    s = s.substring(idx1 + 4);
    idx1 = s.search(/[^= "']/);
    s = s.substring(idx1);
    idx1 = s.search(/[ "']/);

    return s.substring(0, idx1);
}
