#!/usr/bin/env python

import string
import re

words = {
    'WS' : '_ws',
    'EOL' : '_eol',
    'AutoC' : 'autoc',
    'KeyWords' : 'Keywords',
    'BackSpace' : 'Backspace',
    'UnIndents' : 'Unindents',
    'TargetRE' : 'TargetRe'
}

dont_render = []

sci_defs = []
sci_impl = []

def fix_name(name):
    str = ''
    
    for word in words.keys():
        if string.find(name, word) >= 0:
            name = string.replace(name, word, words[word])
    
    for i in range(len(name)):
        if name[i] in string.uppercase:
            if i > 0:
                str = str + '_%c' % string.lower(name[i])
            else:
                str = str + string.lower(name[i])
        else:
            str = str + name[i]
    return str

def write_fun(dict):
    def_full = '%(ret_type)s gtk_scintilla_%(fun_name)s(GtkScintilla *sci, %(first_type)s %(first_name)s, %(second_type)s %(second_name)s)'
    def_first = '%(ret_type)s gtk_scintilla_%(fun_name)s(GtkScintilla *sci, %(first_type)s %(first_name)s)'
    def_second = '%(ret_type)s gtk_scintilla_%(fun_name)s(GtkScintilla *sci, %(second_type)s %(second_name)s)'
    def_none = '%(ret_type)s gtk_scintilla_%(fun_name)s(GtkScintilla *sci)'
    
    body_full = '''
{
    %(return)sscintilla_send_message(SCINTILLA(sci->scintilla),
        %(msg_num)s, (gulong) %(first_name)s, (gulong) %(second_name)s);
}
'''
    body_first = '''
{
    %(return)sscintilla_send_message(SCINTILLA(sci->scintilla),
        %(msg_num)s, (gulong) %(first_name)s, 0);
}
'''
    body_second = '''
{
    %(return)sscintilla_send_message(SCINTILLA(sci->scintilla),
        %(msg_num)s, 0, (gulong) %(second_name)s);
}
'''
    body_none = '''
{
    %(return)sscintilla_send_message(SCINTILLA(sci->scintilla),
        %(msg_num)s, 0, 0);
}
'''
    
    dict['fun_name'] = fix_name(dict['fun_name'])
    if dict['fun_name'] in dont_render:
        return
    
    for key in dict.keys():
        if string.find(key, 'type') > 0:
            if dict[key] == 'void':
                pass
            elif dict[key] == 'int':
                pass
            elif dict[key] == 'bool':
                dict[key] = 'gint'
            elif dict[key] == 'position':
                dict[key] = 'glong'
            elif dict[key] == 'colour':
                dict[key] = 'glong'
            elif dict[key] == 'string':
                dict[key] = 'const gchar *'
            elif dict[key] == 'stringresult':
                dict[key] = 'gchar *'
            elif dict[key] is None:
                pass
            else:
                print 'Function not wrapped: ' + dict['fun_name']
                return
    
    if dict['ret_type'] != 'void':
        dict['return'] = 'return '
    else:
        dict['return'] = ''
    
    if dict['first_name'] is not None:
        dict['first_name'] = fix_name(dict['first_name'])
    if dict['second_name'] is not None:
        dict['second_name'] = fix_name(dict['second_name'])
    
    if dict['first_type'] is None and dict['second_type'] is None:
        fun_def = def_none % dict
        fun_body = body_none % dict
    elif dict['second_type'] is None:
        fun_def = def_first % dict
        fun_body = body_first % dict
    elif dict['first_type'] is None:
        fun_def = def_second % dict
        fun_body = body_second % dict
    else:
        fun_def = def_full % dict
        fun_body = body_full % dict
    
    sci_defs.append(fun_def + ';\n')
    sci_impl.append(fun_def + fun_body + '\n')

def write_val(dict):
    val_template = '#define %(name)s %(value)s'
    
    sci_defs.append(val_template % dict + '\n')

def main():
    iface = open('scintilla/include/Scintilla.iface', 'r')
    
    fun_re = '^(fun|get|set)\s+(?P<ret_type>\S+)\s+(?P<fun_name>\S+)=(?P<msg_num>\d+)?\(((?P<first_type>\w+)\s+(?P<first_name>\w+)\s*)?,(\s*(?P<second_type>\w+)\s+(?P<second_name>\w+))?\)'
    val_re = '^val\s+(?P<name>\w+)\s*=\s*(?P<value>.+)$'
    
    line = iface.readline()
    while line != '':
        if string.find(line, 'cat') == 0 and \
           string.split(string.strip(line))[1] == 'Deprecated':
            break
        
        if re.match(fun_re, line):
            write_fun(re.match(fun_re, line).groupdict())
        elif re.match(val_re, line):
            write_val(re.match(val_re, line).groupdict())
        
        line = iface.readline()
    
    iface.close()
    
    template = string.replace(open('gtkscintilla.c.in').read(),
                              '%scintilla_impl%', string.join(sci_impl, ''))
    open('gtkscintilla.c', 'w').write(template)
    
    template = string.replace(open('gtkscintilla.h.in').read(),
                              '%scintilla_defs%', string.join(sci_defs, ''))
    open('gtkscintilla.h', 'w').write(template)

if __name__ == '__main__':
    main()
