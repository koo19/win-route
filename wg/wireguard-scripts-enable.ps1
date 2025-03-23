param(
  [Parameter(Mandatory = $true)]
  [ValidateSet('add', 'delete')]
  [string]$Action
)

# 检查是否具有管理员权限
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

# 如果不是管理员权限，则使用 sudo 重新运行
if (-not $isAdmin) {
  Start-Process powershell.exe -ArgumentList "-File `"$($MyInvocation.MyCommand.Path)`" $Action" -Verb RunAs
  exit
}

# 注册表操作函数
function Set-WireGuardRegistry {
  param (
    [string]$Action
  )
    
  $registryPath = "HKLM\Software\WireGuard"
  $valueName = "DangerousScriptExecution"
    
  try {
    switch ($Action) {
      'add' {
        Write-Host "Enabling WireGuard script execution..."
        reg add $registryPath /v $valueName /t REG_DWORD /d 1 /f
        Write-Host "Successfully enabled WireGuard script execution" -ForegroundColor Green
      }
      'delete' {
        Write-Host "Disabling WireGuard script execution..."
        reg delete $registryPath /v $valueName /f
        Write-Host "Successfully disabled WireGuard script execution" -ForegroundColor Green
      }
    }
  }
  catch {
    Write-Error "Failed to $Action WireGuard registry: $($_.Exception.Message)"
    exit 1
  }
}

# 执行注册表操作
Set-WireGuardRegistry -Action $Action

# 如果是添加操作，继续检查脚本文件
if ($Action -eq 'add') {
  # 当前脚本所在目录
  $scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

  # 确认所需脚本存在
  $requiredScripts = @(
    "wireguard-hooks.ps1",
    "manage-routes.ps1"
  )

  foreach ($script in $requiredScripts) {
    $scriptPath = Join-Path $scriptDir $script
    if (-not (Test-Path $scriptPath)) {
      Write-Error "Required script not found: $script"
      exit 1
    }
  }

  Write-Host "`nSetup completed successfully!" -ForegroundColor Green
}
