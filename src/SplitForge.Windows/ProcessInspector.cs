using System.Runtime.Versioning;

namespace SplitForge.Platform;

/// <summary>Consultas de procesos de solo lectura (QUERY_LIMITED_INFORMATION / lista de módulos).</summary>
[SupportedOSPlatform("windows")]
public static unsafe class ProcessInspector
{
    /// <summary>Ruta del ejecutable de un proceso. Solo para uso local: nunca se exporta.</summary>
    public static string? ImagePath(int pid)
    {
        if (pid <= 0)
            return null;
        var handle = Native.OpenProcess(Native.PROCESS_QUERY_LIMITED_INFORMATION, 0, (uint)pid);
        if (handle == 0)
            return null;
        try
        {
            var buffer = new char[32768];
            var size = buffer.Length;
            fixed (char* p = buffer)
            {
                return Native.QueryFullProcessImageName(handle, 0, p, ref size) != 0 ? new string(p, 0, size) : null;
            }
        }
        finally
        {
            Native.CloseHandle(handle);
        }
    }

    public static IReadOnlyList<int> AllProcessIds()
    {
        var capacity = 4096;
        while (true)
        {
            var ids = new uint[capacity];
            fixed (uint* p = ids)
            {
                if (Native.EnumProcesses(p, (uint)(ids.Length * sizeof(uint)), out var needed) == 0)
                    return [];
                var count = (int)(needed / sizeof(uint));
                if (count < capacity)
                    return [.. ids.Take(count).Where(id => id != 0).Select(id => (int)id)];
            }
            capacity *= 2;
        }
    }

    /// <summary>PIDs cuyo ejecutable se llama <paramref name="exeName"/> (sin distinguir mayúsculas).</summary>
    public static IReadOnlyList<int> FindByExecutable(string exeName) =>
        [.. AllProcessIds().Where(pid => ImagePath(pid) is { } path
                                         && string.Equals(Path.GetFileName(path), exeName, StringComparison.OrdinalIgnoreCase))];

    /// <summary>Nombres (sin rutas) de los módulos cargados, o null si Windows deniega el acceso (p. ej. con EAC).</summary>
    public static IReadOnlyList<string>? ModuleNames(int pid)
    {
        var handle = Native.OpenProcess(Native.PROCESS_QUERY_INFORMATION | Native.PROCESS_VM_READ, 0, (uint)pid);
        if (handle == 0)
            return null;
        try
        {
            var capacity = 1024;
            nint[] modules;
            int count;
            while (true)
            {
                modules = new nint[capacity];
                fixed (nint* p = modules)
                {
                    if (Native.EnumProcessModulesEx(handle, p, (uint)(modules.Length * sizeof(nint)), out var needed, Native.LIST_MODULES_ALL) == 0)
                        return null;
                    count = (int)(needed / (uint)sizeof(nint));
                }
                if (count <= capacity)
                    break;
                capacity = count;
            }
            var names = new List<string>(count);
            var name = stackalloc char[260];
            for (var i = 0; i < count; i++)
            {
                var len = Native.GetModuleBaseName(handle, modules[i], name, 260);
                if (len > 0)
                    names.Add(new string(name, 0, (int)len));
            }
            return names;
        }
        finally
        {
            Native.CloseHandle(handle);
        }
    }
}
