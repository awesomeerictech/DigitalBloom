

import QtQuick
import QtQuick.Controls.Material
import Fluid as Fluid

Item {
    Fluid.SearchBar {
        id: searchbar
        onSearchTextChanged: {
            if ("fluid".startsWith(searchText)) {
                searchSuggestions.append({text: "fluid"});
                searchSuggestions.append({text: "fluid demo"});
                searchSuggestions.append({text: "fluid interface"});
                searchSuggestions.append({text: "fluid layout"});
            } else if ("liri".startsWith(searchText)) {
                searchSuggestions.append({text: "liri"});
                searchSuggestions.append({text: "liri os"});
            }
        }
        onSearch: {
            searchResults.append({title: "example search result", body: "This text can go over multiple lines and automatically wraps accordingly. This section is used to provide a short description of the search result.", uri: query+"0"})
            searchResults.append({title: "second example search result", body: "Description", uri: query+"1"})
            searchResults.append({title: "third example search result", body: "Description", uri: query+"2"})
        }
    }
    ListView {
        id: resultsListView
        visible: searchbar.searchResults.count !== 0
        anchors.top: searchbar.bottom
        x: searchbar.x
        width: parent.width
        height: parent.height
        model: searchbar.searchResults
        clip: true
        delegate: Item {
            width: parent.width
            height: 128
            Fluid.Card {
                anchors.fill: parent
                anchors.margins: 8
                Fluid.Ripple {
                    anchors.fill: parent
                    onClicked: console.log(model.uri)
                }
                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 16
                    Fluid.TitleLabel {
                        text: model.title
                    }
                    Fluid.BodyLabel {
                        width: parent.width
                        text: model.body
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }
}
