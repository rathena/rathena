# Security Policy

## Supported Versions

This project is currently in active development and should be considered alpha / early beta software.

Only the latest version is supported.

Before using this project in production:

- Test thoroughly in a non-production environment.
- Review source code and configuration.
- Keep dependencies up to date.

Security fixes, if provided, will generally be released only for the latest version.

Development speed and feature implementation may take priority over stability during this stage of the project.

## Reporting a Vulnerability

Please report all security vulnerabilities through GitHub Issues.

This project is open source, actively developed, and currently considered alpha / early beta software. Public disclosure of vulnerabilities is encouraged, as transparency helps identify weaknesses earlier and benefits both contributors and server operators.

There is no private disclosure process at this time.

When reporting a vulnerability, please include:

- Description of the issue
- Steps to reproduce
- Expected and actual behavior
- Impact assessment
- Relevant logs, screenshots, or proof-of-concept material

Security reports, design flaws, protocol weaknesses, anti-cheat bypasses, exploit vectors, implementation bugs, and other security-related findings are all welcome and should be reported through the normal GitHub Issue process.

## Security Philosophy

Security through obscurity is not a goal of this project.

The project is developed entirely in the open, and security discussions are expected to take place publicly whenever possible. Vulnerabilities, design limitations, protocol weaknesses, implementation flaws, and security concerns are considered valuable feedback that helps improve the project.

Users should assume that all client-side behavior can be inspected, modified, reverse engineered, and reproduced. Any security model that depends solely on secrecy, hidden client logic, or undisclosed implementation details should be considered ineffective.

Finding and openly discussing weaknesses is considered part of the development process.

## Security Scope and Limitations

This project is an open-source client implementation.

Because the source code is publicly available, any security weaknesses that originate from the server-side protocol, API design, authentication model, emulator implementation, or overall system architecture may also be visible in this client and cannot be fully mitigated on the client side alone.

If you discover a vulnerability that appears to originate from the server or emulator implementation rather than this project, please report it to the corresponding server/emulator maintainers.

Client-side protections should not be considered a replacement for proper server-side security controls.

Security ultimately depends on the combined design and implementation of the server, protocol, infrastructure, and client software.

## Anti-Cheat and Security Considerations

This project cannot guarantee protection against cheating, botting, automation, packet manipulation, memory modification, client modifications, or other forms of abuse.

The open-source nature of the client means that any client-side protection can be inspected, modified, bypassed, removed, or disabled by users with sufficient technical knowledge.

Effective protection against exploits requires robust server-side validation and security controls. Server operators should not rely solely on client-side security mechanisms.

Any vulnerability that stems from protocol design, server logic, emulator behavior, trust assumptions, or server-side validation weaknesses should be considered a server-side issue and reported to the relevant emulator or server project.

## Mixed Client Environments

Users operating both desktop and web clients should be aware that desktop security solutions may not provide equivalent protection for browser-based clients.

Allowing web clients through client-side security controls, anti-cheat systems, or other security software may reduce the effectiveness of those protections and potentially expose the environment to abuse, automation, botting, modified clients, and other exploit techniques.

Server operators should carefully evaluate the security implications before enabling mixed desktop and web client environments.

## Third-Party Components

This project depends on third-party libraries, frameworks, services, and emulator software.

Security issues originating from third-party software may need to be addressed by their respective maintainers and not by this project.

Reporting such issues publicly is still encouraged so users and maintainers can understand the full security impact on the ecosystem.

## No Security Warranty

This software is provided strictly on an "as is" basis without warranty of any kind.

The maintainers make no guarantee that the software is secure, free from vulnerabilities, suitable for any particular purpose, or that reported issues will be fixed.

Users, server operators, and contributors are solely responsible for evaluating the security, stability, and suitability of this software for their environment.

By using this project, you acknowledge that security issues may exist, may be publicly known, and may remain unresolved for an indefinite period of time.

If complete security, stability, support guarantees, or responsible disclosure processes are required, you should not rely on this software.
