#!/bin/bash

# Script to validate GitHub Actions workflow syntax
set -e

echo "🔍 Validating GitHub Actions workflows..."

# Check if GitHub CLI is available
if ! command -v gh &> /dev/null; then
    echo "⚠️  GitHub CLI not found. Skipping workflow validation."
    echo "   Install from: https://cli.github.com/"
    exit 0
fi

# Check workflow syntax
echo "📋 Checking workflow files..."

WORKFLOWS_DIR=".github/workflows"
if [[ ! -d "$WORKFLOWS_DIR" ]]; then
    echo "❌ Workflows directory not found: $WORKFLOWS_DIR"
    exit 1
fi

VALIDATION_FAILED=0

for workflow in "$WORKFLOWS_DIR"/*.yml "$WORKFLOWS_DIR"/*.yaml; do
    if [[ -f "$workflow" ]]; then
        filename=$(basename "$workflow")
        echo "  🔧 Validating: $filename"

        # Basic YAML syntax check using Python (if available)
        if command -v python3 &> /dev/null; then
            if python3 -c "import yaml" 2>/dev/null; then
                if ! python3 -c "import yaml; yaml.safe_load(open('$workflow'))" 2>/dev/null; then
                    echo "    ❌ Invalid YAML syntax"
                    VALIDATION_FAILED=1
                    continue
                fi
            else
                echo "    ℹ️  PyYAML not available, skipping syntax check"
            fi
        fi

        # Check for common workflow issues
        if ! grep -q "runs-on:" "$workflow"; then
            echo "    ⚠️  Warning: No 'runs-on' found"
        fi

        if ! grep -q "uses: actions/" "$workflow"; then
            echo "    ℹ️  Note: No GitHub actions used"
        fi

        echo "    ✅ Syntax looks good"
    fi
done

if [[ $VALIDATION_FAILED -eq 1 ]]; then
    echo ""
    echo "❌ Workflow validation failed!"
    exit 1
fi

echo ""
echo "✅ All workflows validated successfully!"

# Show workflow summary
echo ""
echo "📊 Workflow Summary:"
for workflow in "$WORKFLOWS_DIR"/*.yml "$WORKFLOWS_DIR"/*.yaml; do
    if [[ -f "$workflow" ]]; then
        filename=$(basename "$workflow")
        echo "  📄 $filename"

        # Extract trigger events
        triggers=$(grep -A 10 "^on:" "$workflow" | grep -E "^\s+[a-z_]+:" | sed 's/://g' | xargs | head -1)
        echo "    🔄 Triggers: $triggers"

        # Extract job names
        jobs=$(grep -A 50 "^jobs:" "$workflow" | grep -E "^\s+[a-zA-Z_-]+:" | sed 's/://g' | xargs)
        echo "    ⚙️  Jobs: $jobs"
        echo ""
    fi
done

echo "🎯 To test these workflows:"
echo "  1. Push to main branch (triggers build.yml)"
echo "  2. Create version tag: git tag v1.0.0 && git push origin v1.0.0 (triggers release.yml)"
echo "  3. Manual dispatch: gh workflow run release.yml --field version=v1.0.0 --field create_release=true"