#include "OcTree.hpp"

void OcTree::AddPoint(const glm::vec3 &point)
{
	// Check if we contain the point
	if (!m_Oct.ContainsPoint(point))
		return;

	// If we contain the point and have room, add the point
	if (m_Points.size() < m_Capacity)
	{
		m_Points.push_back(point);
		return;
	}

	// If we have not subdivided are at capacity, add the point and subdivide
	if (!m_SubDivided)
	{
		m_Points.push_back(point);
		SubDivide();
		m_SubDivided = true;
	}
	
	// If we have subdivided, add the points to the children
	for (const auto &octs : m_Children)
	{
		octs->AddPoint(point);
	}
}

void OcTree::Generate(const uint32_t &size, const std::vector<glm::vec3> points)
{
	// Clear the data for the octree and recreate it
	m_Children.clear();
	m_Points.clear();
	m_SubDivided = false;
	m_Size = size;
	float sizef = (float)size / 2.0f;
	
	glm::vec3 origin = glm::vec3(sizef, sizef, sizef);
	glm::vec3 dim = glm::vec3(sizef, sizef, sizef);
	m_Oct = AABB(origin, dim);

	if (size == 1)
		m_Capacity = 1;
	else
		m_Capacity = 0;

	for (const auto &point : points)
		AddPoint(point);
}

int OcTree::GetOctCount()
{
	// Return the number of Octs generated
	int count = 1;
	for (int i = 0; i < m_Children.size(); i++)
	{
		count += m_Children[i]->GetOctCount();
	}
	return count;
}

void OcTree::GetAllPoints(std::vector<glm::vec3> &points)
{
	// Get the number of points in the scene
	for (const auto &point : m_Points)
	{
		if (m_Size == 1) points.push_back(point);
	}
}

void OcTree::GetAllChildren(std::vector<OcTree*> &children)
{
	// Get the all descendents of the Oct that have points
	if (m_Size == 1 && m_Points.size() == 1)
		children.push_back(this);

	for (const auto &child : m_Children)
	{
		child->GetAllChildren(children);
	}
}

void OcTree::SubDivide()
{
	uint32_t size = m_Size / 2;
	float sizef = (float)m_Size / 4.0f;
	glm::vec3 dims = glm::vec3(sizef, sizef, sizef);

	// Divide up the oct into equally spaced eights
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x - sizef, m_Oct.Origin.y - sizef, m_Oct.Origin.z - sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x + sizef, m_Oct.Origin.y - sizef, m_Oct.Origin.z - sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x - sizef, m_Oct.Origin.y - sizef, m_Oct.Origin.z + sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x + sizef, m_Oct.Origin.y - sizef, m_Oct.Origin.z + sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x - sizef, m_Oct.Origin.y + sizef, m_Oct.Origin.z - sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x + sizef, m_Oct.Origin.y + sizef, m_Oct.Origin.z - sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x - sizef, m_Oct.Origin.y + sizef, m_Oct.Origin.z + sizef), dims), size));
	m_Children.push_back(new OcTree(AABB(glm::vec3(m_Oct.Origin.x + sizef, m_Oct.Origin.y + sizef, m_Oct.Origin.z + sizef), dims), size));
}
