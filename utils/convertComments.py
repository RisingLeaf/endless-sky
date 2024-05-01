import os


paths = ["source/", "source/comparators/", "source/ship/", "source/text/"]

for path in paths:
    directory = os.fsencode(path)
    for file in os.listdir(directory):
        filename = os.fsdecode(file)
        if filename.endswith(".h") or filename.endswith(".cpp"):
            lines = None
            with open(path + filename, 'r') as file:
                lines = file.readlines()
            
            newLines = []
            commentLines = []

            inCommentBlock = False
            identationLevel = 0
            for line in lines:
                prefix = ""
                if filename.endswith(".h"):
                    identationLevel = len(line) - len(line.lstrip('\t'))
                    for x in range(0, identationLevel):
                        prefix += "\t"
    
                if line.startswith(prefix + "//") and not inCommentBlock:
                    inCommentBlock = True
                    commentLines.append(line)
                elif not line.startswith(prefix + "//") and inCommentBlock:
                    inCommentBlock = False
                    newLines.append(prefix + "/**\n")
                    for cLine in commentLines:
                        converted = cLine.replace("//", " *")
                        newLines.append(converted)
                    newLines.append(prefix + "*/\n")
                    newLines.append(line)
                    commentLines.clear()
                elif inCommentBlock:
                    commentLines.append(line)
                else:
                    newLines.append(line)
            
            with open(path + filename, 'w') as file:
                for line in newLines:
                    file.write(line)

