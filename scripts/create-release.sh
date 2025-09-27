#!/bin/bash

# Script to create a GitHub release using the workflow
set -e

usage() {
    echo "Usage: $0 [options] <version>"
    echo "Options:"
    echo "  -h, --help     Show this help message"
    echo "  -d, --draft    Create as draft release"
    echo "  -p, --prerelease Create as prerelease"
    echo "  --no-release   Build binaries without creating release"
    echo ""
    echo "Examples:"
    echo "  $0 v1.0.0                    # Create release v1.0.0"
    echo "  $0 --draft v1.0.0-beta       # Create draft release"
    echo "  $0 --no-release v1.0.0       # Build binaries only"
}

VERSION=""
CREATE_RELEASE="true"
DRAFT="false"
PRERELEASE="false"

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -d|--draft)
            DRAFT="true"
            shift
            ;;
        -p|--prerelease)
            PRERELEASE="true"
            shift
            ;;
        --no-release)
            CREATE_RELEASE="false"
            shift
            ;;
        v*)
            VERSION="$1"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

if [[ -z "$VERSION" ]]; then
    echo "Error: Version is required"
    usage
    exit 1
fi

# Validate version format
if [[ ! "$VERSION" =~ ^v[0-9]+\.[0-9]+\.[0-9]+.*$ ]]; then
    echo "Error: Version must be in format v1.2.3 (with optional suffix like -beta)"
    exit 1
fi

echo "Creating release for version: $VERSION"
echo "Create release: $CREATE_RELEASE"
echo "Draft: $DRAFT"
echo "Prerelease: $PRERELEASE"

# Check if we're in a git repository
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo "Error: Not in a git repository"
    exit 1
fi

# Check if GitHub CLI is installed
if ! command -v gh &> /dev/null; then
    echo "Error: GitHub CLI (gh) is not installed"
    echo "Install it from: https://cli.github.com/"
    exit 1
fi

# Check if user is authenticated
if ! gh auth status &> /dev/null; then
    echo "Error: Not authenticated with GitHub CLI"
    echo "Run: gh auth login"
    exit 1
fi

# Check for uncommitted changes
if ! git diff --quiet; then
    echo "Warning: You have uncommitted changes"
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Trigger workflow dispatch
echo "Triggering release workflow..."

gh workflow run release.yml \
    --field version="$VERSION" \
    --field create_release="$CREATE_RELEASE"

echo "✅ Release workflow triggered successfully!"
echo ""
echo "You can monitor the progress at:"
echo "https://github.com/$(gh repo view --json owner,name -q '.owner.login + "/" + .name')/actions"
echo ""

if [[ "$CREATE_RELEASE" == "true" ]]; then
    echo "When complete, the release will be available at:"
    echo "https://github.com/$(gh repo view --json owner,name -q '.owner.login + "/" + .name')/releases/tag/$VERSION"
fi