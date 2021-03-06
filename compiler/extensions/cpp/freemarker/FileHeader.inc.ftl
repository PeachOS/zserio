<#macro file_header generatorDescription>
/**
 * Automatically generated by ${generatorDescription}.
 */
</#macro>

<#macro camel_case_to_underscores value>
    <#t>${value?replace("(?<=[a-z0-9])[A-Z]", "_$0", "r")?upper_case}
</#macro>

<#macro include_guard_name packagePath typeName>
    <#list packagePath as namespace>
        <#t><@camel_case_to_underscores namespace/>_<#rt>
    </#list>
    <#t><@camel_case_to_underscores typeName/>_H
</#macro>

<#macro include_path packagePath typeName>
    <#list packagePath as namespace>
        <#t>${namespace}/<#rt>
    </#list>
    <#t>${typeName}
</#macro>

<#macro system_includes includeFiles, autoNewLine>
    <#list includeFiles as include>
#include <${include}>
    </#list>
    <#if includeFiles?has_content && autoNewLine>

    </#if>
</#macro>

<#macro user_includes includeFiles, autoNewLine>
    <#list includeFiles as include>
#include "${include}"
    </#list>
    <#if includeFiles?has_content && autoNewLine>

    </#if>
</#macro>

<#macro include_guard_begin packagePath typeName>
#ifndef <@include_guard_name packagePath, typeName/>
#define <@include_guard_name packagePath, typeName/>
</#macro>

<#macro include_guard_end packagePath typeName>
#endif // <@include_guard_name packagePath, typeName/>
</#macro>

<#macro namespace_begin packagePath>
    <#list packagePath as namespace>
namespace ${namespace}
{
    </#list>
</#macro>

<#macro namespace_end packagePath>
    <#list packagePath?reverse as namespace>
} // namespace ${namespace}
    </#list>
</#macro>

<#macro anonymous_namespace_begin>
namespace
{
</#macro>

<#macro anonymous_namespace_end>
} // namespace
</#macro>
