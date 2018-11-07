package zserio.emit.python.types;

import java.util.Arrays;
import java.util.List;

import zserio.emit.common.NativeType;

public class PythonNativeType implements NativeType
{
    public PythonNativeType()
    {
        this(null, null);
    }

    protected PythonNativeType(List<String> packagePath, String name)
    {
        this.packagePath = packagePath;
        this.name = name;
    }

    @Override
    public String getFullName()
    {
        return String.join(".", Arrays.asList(getPackagePath(), name));
    }

    @Override
    public String getName()
    {
        return name;
    }

    public String getPackagePath()
    {
        return String.join(".", packagePath);
    }

    private final List<String> packagePath;
    private final String name;
}