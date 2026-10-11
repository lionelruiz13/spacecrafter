# The one C++ comment scanner of the shape instruments (names.py, writers.py, codeident.py): one pass, in source order.


def decomment(text, literals=True):  # Return text with each comment as one space; literals kept, or emptied to "" / ' ' when literals is False
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if c in '"\'':
            j = i + 1
            while j < n and text[j] != c and text[j] != '\n':
                j += 2 if text[j] == '\\' else 1
            end = j + 1 if j < n and text[j] == c else min(j, n)   # an unterminated literal ends with its line
            out.append(text[i:end] if literals else c + ' ' + c if c == "'" else '""')
            i = end
        elif text.startswith('//', i):
            j = text.find('\n', i); j = n if j < 0 else j
            while text[j - 1] == '\\' and j < n:          # line-continued // comment
                j = text.find('\n', j + 1); j = n if j < 0 else j
            out.append(' '); i = j
        elif text.startswith('/*', i):
            j = text.find('*/', i + 2); j = n if j < 0 else j + 2
            out.append(' '); i = j
        else:
            out.append(c); i += 1
    return ''.join(out)
