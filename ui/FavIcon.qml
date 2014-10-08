import QtQuick 2.3
import "FavIcon.js" as FavIcon

//Takes the URL of a website and tries to retrieve its favicon manually
//It's because of damaged icon handling in WebView. Can be removed when
//WebKit or WebEngine will fix that problem

Image {
    property string site

    function onResponse(xhr) {
        if (xhr.readyState === XMLHttpRequest.DONE) {
            var icon = FavIcon.favIcon(xhr.responseText)
            if(icon !== "") {
                if(icon.indexOf("//") === 0) icon = "http:" + icon
                if(icon.indexOf("http://") !== -1 || icon.indexOf("https://") !== -1)
                    source = icon
                else source = FavIcon.host(site) + icon
            } else {
                source = ""
            }
        }
    }

    onSiteChanged: {
        var closure = function(xhr) { return function() { onResponse(xhr); }};
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = closure(xhr);
        xhr.open("GET", FavIcon.host(site), true);
        xhr.send();
    }
}
