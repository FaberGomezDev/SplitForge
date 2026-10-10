using System.Text;

namespace SplitForge.Core.Steam;

/// <summary>Nodo del formato KeyValues de Valve (VDF/ACF). Las claves no distinguen mayúsculas.</summary>
public sealed class KvNode
{
    private readonly List<KvNode> _children = [];

    public KvNode(string key, string? value = null)
    {
        Key = key;
        Value = value;
    }

    public string Key { get; }

    /// <summary>Valor de una hoja; <c>null</c> si el nodo es una sección.</summary>
    public string? Value { get; }

    public IReadOnlyList<KvNode> Children => _children;

    public KvNode? Child(string key) =>
        _children.FirstOrDefault(c => string.Equals(c.Key, key, StringComparison.OrdinalIgnoreCase));

    /// <summary>Valor de la primera hoja hija con esa clave.</summary>
    public string? this[string key] => Child(key)?.Value;

    internal void Add(KvNode child) => _children.Add(child);
}

/// <summary>Lector tolerante de KeyValues: nunca ejecuta nada y se detiene ante datos corruptos.</summary>
public static class KeyValuesParser
{
    public const int MaxDepth = 32;
    public const int MaxLength = 4 * 1024 * 1024;

    /// <summary>Devuelve una raíz sintética (clave vacía) con los nodos de primer nivel.</summary>
    public static KvNode Parse(string text)
    {
        var root = new KvNode(string.Empty);
        if (text.Length > MaxLength)
            text = text[..MaxLength];
        var reader = new Reader(text);
        ParseInto(root, ref reader, depth: 0);
        return root;
    }

    private static void ParseInto(KvNode parent, ref Reader reader, int depth)
    {
        while (reader.Next(out var token))
        {
            if (token.Kind == TokenKind.Close)
                return;
            if (token.Kind == TokenKind.Open)
                continue; // Llave sin clave: se ignora.
            var key = token.Text;
            if (!reader.Next(out var next))
                return;
            if (next.Kind == TokenKind.Open)
            {
                var section = new KvNode(key);
                parent.Add(section);
                if (depth + 1 >= MaxDepth)
                    return;
                ParseInto(section, ref reader, depth + 1);
            }
            else if (next.Kind == TokenKind.String)
            {
                parent.Add(new KvNode(key, next.Text));
                reader.SkipConditional();
            }
            else
            {
                return;
            }
        }
    }

    private enum TokenKind { String, Open, Close }

    private readonly record struct Token(TokenKind Kind, string Text);

    private ref struct Reader(string text)
    {
        private readonly string _text = text;
        private int _pos;

        public bool Next(out Token token)
        {
            SkipTrivia();
            token = default;
            if (_pos >= _text.Length)
                return false;
            var c = _text[_pos];
            switch (c)
            {
                case '{':
                    _pos++;
                    token = new Token(TokenKind.Open, "{");
                    return true;
                case '}':
                    _pos++;
                    token = new Token(TokenKind.Close, "}");
                    return true;
                case '"':
                    token = new Token(TokenKind.String, ReadQuoted());
                    return true;
                default:
                    token = new Token(TokenKind.String, ReadBare());
                    return true;
            }
        }

        /// <summary>Ignora condiciones de plataforma como <c>[$WIN32]</c> tras un valor.</summary>
        public void SkipConditional()
        {
            var save = _pos;
            while (save < _text.Length && _text[save] is ' ' or '\t')
                save++;
            if (save < _text.Length && _text[save] == '[')
            {
                var end = _text.IndexOf(']', save);
                _pos = end < 0 ? _text.Length : end + 1;
            }
        }

        private void SkipTrivia()
        {
            while (_pos < _text.Length)
            {
                var c = _text[_pos];
                if (char.IsWhiteSpace(c) || c == '﻿')
                {
                    _pos++;
                }
                else if (c == '/' && _pos + 1 < _text.Length && _text[_pos + 1] == '/')
                {
                    var end = _text.IndexOf('\n', _pos);
                    _pos = end < 0 ? _text.Length : end + 1;
                }
                else
                {
                    return;
                }
            }
        }

        private string ReadQuoted()
        {
            _pos++; // comilla inicial
            var sb = new StringBuilder();
            while (_pos < _text.Length)
            {
                var c = _text[_pos++];
                if (c == '"')
                    return sb.ToString();
                if (c == '\\' && _pos < _text.Length)
                {
                    var e = _text[_pos++];
                    switch (e)
                    {
                        case 'n': sb.Append('\n'); break;
                        case 't': sb.Append('\t'); break;
                        case '\\': sb.Append('\\'); break;
                        case '"': sb.Append('"'); break;
                        default: sb.Append('\\').Append(e); break;
                    }
                }
                else
                {
                    sb.Append(c);
                }
            }
            return sb.ToString();
        }

        private string ReadBare()
        {
            var start = _pos;
            while (_pos < _text.Length && !char.IsWhiteSpace(_text[_pos]) && _text[_pos] is not ('{' or '}' or '"'))
                _pos++;
            return _text[start.._pos];
        }
    }
}
