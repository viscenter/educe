import sys

def genFileNames():
    names = []
    names.append("ALGOArryEngn_532229600_LCELID_FS.cc")
    names.append("ALGOArryEngn_936873123_LCELID_FS.cc")
    names.append("ALGOArryObjctFldCrtAlgT.GFldLtVlMhHxTLLgnPtHxTLLgndblFDt3ddblLtVlMhHxTLLgnPt.cc")
    names.append("ALGOArryObjctFldDtSclrAlgT.GFldLtVlMhHxTLLgnPtHxTLLgndblFDt3ddblLtVlMhHxTLLgnPt.LtVlMhHxTLLgnPtNd.cc")
    names.append("ALGOArryObjctFldElmPtAlgT.GFldLtVlMhHxTLLgnPtHxTLLgndblFDt3ddblLtVlMhHxTLLgnPt.cc")
    names.append("ALGOArryObjctFldLctnNdAlgT.GFldLtVlMhHxTLLgnPtHxTLLgndblFDt3ddblLtVlMhHxTLLgnPt.LtVlMhHxTLLgnPtNd.cc")
    names.append("MpFldDtFrmSrcTDstntnAlgT.GFldLtVlMhHxTLLgnPtHxTLLgndblFDt3ddblLtVlMhHxTLLgnPt.LtVlMhHxTLLgnPtNd.GFldTrSrfMhTrLnrLgnPtTrLnrLgndblvctrdbl.TrSrfMhTrLnrLgnPtNd.cc")
    names.append("MrchngCbs.GFldLtVlMhHxTLLgnPtHxTLLgndblFDt3ddblLtVlMhHxTLLgnPt.cc")
    names.append("SFIntrfcMkr.GFldLtVlMhHxTLLgnPtHxTLLgndblFDt3ddblLtVlMhHxTLLgnPt.LtVlMhHxTLLgnPtNd.cc")
    return names

def printNames():
    names = genFileNames()
    if len(names) < 1:
        sys.stderr.write("ERROR: Empty file list")
        return 1
    nameString = names[0]
    for name in names[1:]:
        nameString += ";%s" % (name,)
    print nameString
    return 0

def genFiles():
    names = genFileNames()
    import shutil
    for file in names:
        print "Copying %s" % (file,)
        shutil.copy("../../src/DynamicModules/files/%s" % (file,), ".")
    return 0

def main():

    index = 1
    for arg in sys.argv[1:]:
        if arg == "names":
            return printNames()
        print "arg[%s] = %s" % (index, arg)
        index += 1

    return genFiles()

if __name__ == "__main__":
    returnVal= main()
    sys.exit(returnVal)
