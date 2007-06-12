#!/usr/bin/sed -f
# rename a few projects because of the extremly dumb svndumpfilter
s/\(^Node-.*: \)004\ intranet\ application/\1p01-004-intranet-application/
s/\(^Node-.*: \)bananabay-at/\1p02-bananabay-at/
s/\(^Node-.*: \)fckoeln\ new\ design/\1p03-fckoeln-new-design/
s/\(^Node-.*: \)fckoeln\ old\ design/\1p04-fckoeln-old-design/
s/\(^Node-.*: \)vertixx-info/\1p05-vertixx-info/
s/\(^Node-.*: \)vertixx/\1p06-vertixx/
s/\(^Node-.*: \)Dev\ Management/\1p07-Dev-Management/
s/\(^Node-.*: \)Development\ Department/\1p08-Development-Department/
s/\(^Node-.*: \)IT\ Department/\1p09-IT-Department/
s/\(^Node-.*: \)html\ emails/\1p10-html-emails/
s/\(^Node-.*: \)olution\ framework/\1p11-olution-framework/
s/\(^Node-.*: \)search\ engines/\1p12-search-engines/
