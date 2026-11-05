def uwuify(text):
    text = text.replace("r", "w")
    text = text.replace("l", "w")

    text = text.replace("R", "W")
    text = text.replace("L", "W")

    result = []
    for i, c in enumerate(text):
        result.append(c)
        if c in "nN" and i + 1 < len(text) and text[i + 1] in "aeiouyàâäéèêëîïôöùûüAEIOUYÀÂÄÉÈÊËÎÏÔÖÙÛÜ":
            result.append("Y" if text[i + 1].isupper() else "y")
    text = "".join(result)

    text = (text.replace("ove", "uv"))
    text = (text.replace("Ove", "Uv"))
    text = (text.replace("OVE", "UV"))

    return text
